// __!!rengine_copyright!!__ //

#include <rengine/lang/debug/Debug.h>
#include <rengine/lang/exception/Exception.h>

#include <cstdlib>

#if (RENGINE_COMPILER == RENGINE_COMPILER_GNUC)
	#include <cxxabi.h>
	#include <stdio.h>
	#include <execinfo.h>

	#ifdef RENGINE_WITH_BFD
		#include <unistd.h>
		#include <linux/limits.h>
		#include <dlfcn.h>
		#include <link.h>
		#include <bfd.h>
	#endif
#endif

#if (RENGINE_COMPILER == RENGINE_COMPILER_MSVC)
	#include <sstream>
	#include <windows.h>
	#include <dbghelp.h>
	#include <ctime>

	#pragma comment(lib, "Dbghelp.lib")
#endif


namespace rengine
{

#if (RENGINE_COMPILER == RENGINE_COMPILER_MSVC)

	LONG WINAPI exceptionHandler(struct _EXCEPTION_POINTERS *exception_pointer)
	{ 
		LONG retval = EXCEPTION_CONTINUE_SEARCH;

		char app_path[MAX_PATH];
		::GetModuleFileName(0, app_path, MAX_PATH);
		std::string application_name(app_path);
		application_name = application_name.substr(application_name.rfind("\\") + 1);

		std::stringstream dump_path;
		dump_path << application_name << "_" << time(0) << ".dmp";

		HANDLE file = ::CreateFile(dump_path.str().c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file != INVALID_HANDLE_VALUE)
		{
			_MINIDUMP_EXCEPTION_INFORMATION this_exception_info;

			this_exception_info.ThreadId = ::GetCurrentThreadId();
			this_exception_info.ExceptionPointers = exception_pointer;
			this_exception_info.ClientPointers = NULL;

			// write the dump
			if(MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal, &this_exception_info, NULL, NULL))
			{
				//retval = EXCEPTION_EXECUTE_HANDLER;
			}

		}
		::CloseHandle(file);

		return retval;
	}

	void enableApplicationDebugger()
	{
		//_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		::SetUnhandledExceptionFilter( exceptionHandler );
	}

	void createStackTrace(StackMemoryAddress* stack, Uint const size, Uint const skip)
	{
		memset(stack, 0, sizeof(StackMemoryAddress) * size);
		CaptureStackBackTrace(skip, size, (void**) (stack), 0); 	//FramesToSkip, FramesToCapture, BackTrace, BackTraceHash
	}

	void resolverSetup()
	{
		SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
		HANDLE hProcess = GetCurrentProcess();
		SymInitialize(hProcess, NULL, TRUE);
	}

	void resolverTeardown()
	{
		HANDLE hProcess = GetCurrentProcess();
		SymCleanup(hProcess);
	}

	SourceCodeLocation resolveAddress(void* address)
	{
		HANDLE hProcess = GetCurrentProcess();

		//
		// get the symbol info
		//
		DWORD64  dwDisplacement = 0;
		DWORD64  dwAddress = (DWORD64)address;

		unsigned const int size = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR);
		char buffer[size];
		char undecoratedName[size];

		memset(buffer, 0, size);
		memset(undecoratedName, 0, size);

		PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = MAX_SYM_NAME;

		SourceCodeLocation location;

		if (SymFromAddr(hProcess, dwAddress, &dwDisplacement, pSymbol))
		{
			if (UnDecorateSymbolName(pSymbol->Name, undecoratedName, size, UNDNAME_COMPLETE))
			{
				location.function  = _strdup(pSymbol->Name);
				location.function_pretty = _strdup(undecoratedName);
			}
		}

	
		IMAGEHLP_LINE64 line;

		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
		DWORD pdwDisplacement = 0;
		if (SymGetLineFromAddr64(hProcess, dwAddress, &pdwDisplacement, &line))
		{
			// SymGetLineFromAddr64 returned success
			location.filename = _strdup(line.FileName);
			location.line_number = line.LineNumber;
		}
		
		return location;
	}


#elif  (RENGINE_COMPILER == RENGINE_COMPILER_GNUC)

#ifdef RENGINE_WITH_BFD

#define MAX_MODULES 100

	struct ModuleData
	{
		ModuleData()
		{
			file = 0;

			abfd = 0;
			syms = 0;

			section_filename = 0;
			section_functionname = 0;
			section_line = 0;
			section_found = 0;
			section_pc = 0;
		}

		const char* file;


		bfd *abfd;
		asymbol **syms;

		bfd_vma section_pc;
		const char *section_filename;
		const char *section_functionname;
		unsigned int section_line;
		int section_found;
	};

	struct ResolverData
	{
		ResolverData()
		{
			counter = 0;
			memset(modules, 0, sizeof(ModuleData) * MAX_MODULES);
			memset(&self, 0, sizeof(dl_phdr_info));
		}

		Uint counter;
		ModuleData modules[MAX_MODULES];


		dl_phdr_info self;
	};


	static ResolverData resolver_data;

	void shutDownModule(Uint index)
	{
		if (resolver_data.modules[index].syms)
		{
			free(resolver_data.modules[index].syms);
			resolver_data.modules[index].syms = 0;
		}

		if (resolver_data.modules[index].abfd)
		{
			bfd_close(resolver_data.modules[index].abfd);
			resolver_data.modules[index].abfd = 0;
		}

		memset(&resolver_data.modules[index], 0, sizeof(ModuleData));
	}

	static void findAddressInSection(bfd *abfd, asection *section, void *data)
	{
		bfd_vma vma;
		bfd_size_type size;

		ModuleData* r_data = (ModuleData*)(data);

		if (r_data->section_found)
			return;

		if ((bfd_get_section_flags(abfd, section) & SEC_ALLOC) == 0)
			return;

		vma = bfd_get_section_vma(abfd, section);
		if (r_data->section_pc < vma)
			return;

		size = bfd_section_size(abfd, section);
		if (r_data->section_pc >= vma + size)
			return;

		r_data->section_found = bfd_find_nearest_line(abfd, section, r_data->syms, r_data->section_pc - vma - 1,
													  &(r_data->section_filename), &(r_data->section_functionname), &(r_data->section_line));
	}

	struct FileFinder
	{
		FileFinder() :address(0), file(0) {}

		void* address;
		const char* file;
	};

	static int findModuleFile(struct dl_phdr_info *info, size_t size, void *data)
	{
		FileFinder* found = (FileFinder*)(data);

		const ElfW(Phdr) *phdr = info->dlpi_phdr;
		ElfW(Addr) load_base = info->dlpi_addr;

		for (long n = info->dlpi_phnum; --n >= 0; phdr++)
		{
			if (phdr->p_type == PT_LOAD)
			{
				ElfW(Addr) vaddr = phdr->p_vaddr + load_base;
				if ((long unsigned)(found->address) >= vaddr &&
					(long unsigned)(found->address) < vaddr + phdr->p_memsz)
				{
					found->file = info->dlpi_name;
				}
			}
		}

		return 0;
	}

	static int setupModule(struct dl_phdr_info *info, size_t size, void *data)
	{
		ResolverData *resolver = (ResolverData *)data;

		Uint currentModule = resolver->counter++;
		if (currentModule >= MAX_MODULES)
		{
			return 1;
		}


		resolver->modules[currentModule].file = info->dlpi_name;

		resolver->modules[currentModule].abfd = bfd_openr(resolver->modules[currentModule].file, NULL);

		char **matching = 0;
		if (!resolver->modules[currentModule].abfd ||
			bfd_check_format(resolver->modules[currentModule].abfd, bfd_archive) ||
			!bfd_check_format_matches(resolver->modules[currentModule].abfd, bfd_object, &matching) ||
			((bfd_get_file_flags(resolver->modules[currentModule].abfd) & HAS_SYMS) == 0))
		{
			shutDownModule(currentModule);
			resolver->counter--;
			return 0;
		}

		// read symbols
		Uint symsize;
		Long symcount = bfd_read_minisymbols(resolver->modules[currentModule].abfd, false, (void **) &(resolver->modules[currentModule].syms), &symsize);

		if (symcount == 0)
			symcount = bfd_read_minisymbols(resolver->modules[currentModule].abfd, true /* dynamic */ ,(void **) &(resolver->modules[currentModule].syms), &symsize);

		if (symcount < 0)
		{
			shutDownModule(currentModule);
			resolver->counter--;
			return 0;
		}


		return 0;
	}

	void resolverSetup()
	{
		resolverTeardown();

		//
		// application filename
		//
		char id[PATH_MAX];
		char filename[PATH_MAX];
		memset(id, 0, PATH_MAX);
		memset(filename, 0, PATH_MAX);

		sprintf(id, "/proc/%d/exe", getpid());
		readlink(id, filename, PATH_MAX - 1);



		bfd_init();

		// [0] id the executable
		resolver_data.counter = 0;
		resolver_data.self.dlpi_name = rg_strdup(filename);
		setupModule(&resolver_data.self, sizeof(resolver_data.self), &resolver_data);


		resolver_data.counter = 1;
		dl_iterate_phdr(setupModule, &resolver_data);


	}


	void resolverTeardown()
	{
		for (Uint i = 0; i != MAX_MODULES; ++i)
		{
			shutDownModule(i);
		}

		if (resolver_data.self.dlpi_name)
		{
			rg_free((void*)resolver_data.self.dlpi_name);
			resolver_data.self.dlpi_name = 0;
		}


		memset(&resolver_data.self, 0, sizeof(dl_phdr_info));
		resolver_data.counter = 0;
	}


	SourceCodeLocation resolveAddress(void* address)
	{
		SourceCodeLocation location;

		if (resolver_data.modules[0].abfd)
		{
			FileFinder finder;
			finder.address = address;

			dl_iterate_phdr(findModuleFile, &finder);
			if (! (finder.file && strlen(finder.file)) )
			{
				finder.file = resolver_data.modules[0].file;
			}

			if (finder.file)
			{
				for (Uint i = 0; i != MAX_MODULES; ++i)
				{
					if (resolver_data.modules[i].abfd &&
						(strcmp(finder.file, resolver_data.modules[i].file) == 0))
					{
						resolver_data.modules[i].section_pc = (bfd_vma)(address);

						bfd_map_over_sections(resolver_data.modules[i].abfd, findAddressInSection, &(resolver_data.modules[i]) );

						if (resolver_data.modules[i].section_found)
						{
							char *demangled = 0;
			                char *name = (char*) resolver_data.modules[i].section_functionname;
			                if (name)
			                {
			                	int status = 0;
			                	demangled = abi::__cxa_demangle(name, 0, 0, &status);

			                	if (demangled)
			                	{
			                		name = demangled;
			                	}
			                }

							location = SourceCodeLocation(resolver_data.modules[i].section_line, resolver_data.modules[i].section_filename, name, name);

							if (demangled)
							{
								free(demangled);
							}
						}

						resolver_data.modules[i].section_found = 0;
						resolver_data.modules[i].section_line = 0;
						resolver_data.modules[i].section_filename = 0;
						resolver_data.modules[i].section_functionname = 0;
						resolver_data.modules[i].section_pc = 0;
					}
				}
			}

		}

		return location;
	}


#else //RENGINE_WITH_BFD

	void resolverSetup() {}
	void resolverTeardown() {}
	SourceCodeLocation resolveAddress(void* address) { return SourceCodeLocation(); }

#endif //RENGINE_WITH_BFD


	void createStackTrace(StackMemoryAddress* stack, Uint const size, Uint const skip)
	{
		Uint temporarySize = size + skip;
		StackMemoryAddress* temporary = (StackMemoryAddress*) malloc(sizeof(StackMemoryAddress) * temporarySize);

		memset(temporary, 0, sizeof(StackMemoryAddress) * temporarySize);
		backtrace((void**) (temporary), temporarySize);

		memcpy(stack, (void*)(temporary + skip), sizeof(StackMemoryAddress) * size);
		free(temporary);
	}

	void enableApplicationDebugger() { }


#else
	void enableApplicationDebugger() { }
	void createStackTrace(StackMemoryAddress* stack, Uint const size, Uint const skip) { memset(stack, 0, sizeof(StackMemoryAddress) * size); }
	void resolverSetup() {}
	void resolverTeardown() {}
	SourceCodeLocation resolveAddress(void* address) { return SourceCodeLocation(); }
#endif


} //namespace rengine
