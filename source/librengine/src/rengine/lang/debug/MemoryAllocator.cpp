// __!!rengine_copyright!!__ //

#include <rengine/lang/debug/MemoryAllocator.h>
#include <rengine/lang/debug/Debug.h>

#include <cstring>
#include <cstdarg>

#define MEMORY_INT_PRINT_MASK "%15lli"
#define STACK_TRACE_SKIP 4


#if RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32

#include <windows.h>

struct rengine::MemoryAllocator::MemoryMutex
{
	void create()
	{
		InitializeCriticalSection(&critical_section);
	}

	void destroy()
	{
		DeleteCriticalSection(&critical_section);

	}

	void lock()
	{
		EnterCriticalSection(&critical_section);
	}

	void unlock()
	{
		LeaveCriticalSection(&critical_section);
	}

	CRITICAL_SECTION critical_section;
};

#define REPORT_OUTPUT_FILE "logs\\memory_check_up.log"
#else

#include <pthread.h>

struct rengine::MemoryAllocator::MemoryMutex
{
	void create()
	{
		pthread_mutexattr_t mutex_attribute;
		pthread_mutexattr_init(&mutex_attribute);
		pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&mutex, &mutex_attribute);
	}

	void destroy()
	{
		pthread_mutex_destroy(&mutex);

	}
	void lock()
	{
		pthread_mutex_lock(&mutex);
	}

	void unlock()
	{
		pthread_mutex_unlock(&mutex);
	}

	pthread_mutex_t mutex;
};

#define REPORT_OUTPUT_FILE "logs/memory_check_up.log"
#endif //RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32



#define HASH_FUNCTION(address) (HashIterator(address) % HashIterator(hash_table_size))

namespace rengine
{
	static void createStackTrace(MemoryAllocator::ConstPointer stack[MemoryAllocator::stack_size])
	{
		rengine::createStackTrace((StackMemoryAddress*)(stack), MemoryAllocator::stack_size, STACK_TRACE_SKIP);
	}

	class MemoryMutexScopedLock
	{
	public:
		MemoryMutexScopedLock(MemoryAllocator::MemoryMutex& mutex) :mutex_(mutex) { mutex_.lock(); }
		~MemoryMutexScopedLock() { mutex_.unlock();}
	private:
		MemoryAllocator::MemoryMutex& mutex_;
	};

	static const char* OperationTypeNames[] =
	{
			"new",
			"delete",
			"new[]",
			"delete[]",
			"malloc",
			"free"
	};

	static const char* ProblemTypenames[] =
	{
			"Memory Leak",
			"Release Unallocated Memory",
			"Mismatch Memory Allocation/Deletion",
			"Request Zero Memory",
			"Memory Over-Write",
			"Multi-Allocation For Pointer",
	};

	MemoryAllocator* MemoryAllocator::instance()
	{
		static MemoryAllocator allocator;
		return &allocator;
	}

	MemoryAllocator::MemoryAllocator()
		:m_enabled(false),
		 m_memory(0),
		 m_problems(0),
		 m_output(0)
	{

		memset(m_memory_stats, 0, operation_types * sizeof(MemoryStats));
		memset(&m_memory_stats_new, 0, sizeof(MemoryStats));
		memset(&m_memory_stats_new_array, 0, sizeof(MemoryStats));
		memset(&m_memory_stats_malloc, 0, sizeof(MemoryStats));

		//
		// Operations HashTable
		//
		m_memory = (OperationInfo**) malloc(hash_table_size * sizeof(OperationInfo*));
		if (!m_memory)
		{
			return;
		}
		memset(m_memory, 0, hash_table_size * sizeof(OperationInfo*));


		m_mutex = (MemoryMutex*) malloc(sizeof(MemoryMutex));
		if (!m_mutex)
		{
			return;
		}
		memset(m_mutex, 0, sizeof(MemoryMutex));
		m_mutex->create();


		memset(m_null_stack, 0, stack_size * sizeof(ConstPointer));

		m_output = fopen(REPORT_OUTPUT_FILE, "w");
		if (!m_output)
		{
			return;
		}

		enable(true);
	}

	MemoryAllocator::~MemoryAllocator()
	{
		enable(false);

		analyzeMemory();
		reportMemoryStatus();

		if (m_output)
		{
			fclose(m_output);
			m_output = 0;
		}

		if (m_memory)
		{
			emptyMemory();
			free(m_memory);
			m_memory = 0;
		}

		emptyProblemList(&m_problems);

		if (m_mutex)
		{
			m_mutex->destroy();
			free(m_mutex);
			m_mutex = 0;
		}
	}

	void MemoryAllocator::analyzeMemory()
	{
		//
		// Memory Stats
		//
		for (unsigned int i = 0; i != operation_types; ++i)
		{
			switch (i)
			{
			case NewType:
				{
					m_memory_stats_new.count += m_memory_stats[i].count;
					m_memory_stats_new.total_memory += m_memory_stats[i].total_memory;
				}
				break;
			case DeleteType:
				{
					m_memory_stats_new.count -= m_memory_stats[i].count;
					m_memory_stats_new.total_memory -= m_memory_stats[i].total_memory;
				}
				break;
			case NewArrayType:
				{
					m_memory_stats_new_array.count += m_memory_stats[i].count;
					m_memory_stats_new_array.total_memory += m_memory_stats[i].total_memory;
				}
				break;
			case DeleteArrayType:
				{
					m_memory_stats_new_array.count -= m_memory_stats[i].count;
					m_memory_stats_new_array.total_memory -= m_memory_stats[i].total_memory;
				}
				break;
			case MallocType:
				{
					m_memory_stats_malloc.count += m_memory_stats[i].count;
					m_memory_stats_malloc.total_memory += m_memory_stats[i].total_memory;
				}
				break;
			case FreeType:
				{
					m_memory_stats_malloc.count -= m_memory_stats[i].count;
					m_memory_stats_malloc.total_memory -= m_memory_stats[i].total_memory;
				}
				break;

			default:
				break;
			}
		}

		//
		// Detect Memory Leaks
		//
		for (HashIterator hash_index = 0; hash_index != hash_table_size; ++hash_index)
		{
			OperationInfo* iterator = m_memory[hash_index];
			while(iterator)
			{
				ProblemInfo* problem = findProblem(&m_problems, LeakType, iterator->stack, m_null_stack);
				if (!problem)
				{
					problem = createProblem();
					problem->type = LeakType;
					memcpy(problem->allocation_stack, iterator->stack, stack_size * sizeof(ConstPointer));
					addProblemToList(&m_problems, problem);
				}
				problem->count += 1;
				problem->size = iterator->size;
				problem->total_memory += iterator->size;

				iterator = iterator->next;
			}
		}
	}

	void MemoryAllocator::write(char const* message, ...)
	{
		if (m_output == 0) return;

		va_list argp;
		va_start(argp, message);

		vfprintf(m_output, message, argp);
		va_end(argp);
	}

	void MemoryAllocator::reportMemoryStatus()
	{
		write("DynamicCheckUp Memory Report\n");
		write("----------------------------------------------------------------\n");
		write("%15s %15s %15s %15s\n", "->", "operations", "total mem", "max value");

		for (unsigned int i = 0; i != operation_types; ++i)
		{

			write("%15s " MEMORY_INT_PRINT_MASK " " MEMORY_INT_PRINT_MASK " " MEMORY_INT_PRINT_MASK "\n", OperationTypeNames[i],
					m_memory_stats[i].count,
					m_memory_stats[i].total_memory,
					m_memory_stats[i].max_value);
		}

		write("\nDynamic Memory Balance\n");
		write("----------------------------------------------------------------\n");
		write("%15s " MEMORY_INT_PRINT_MASK " " MEMORY_INT_PRINT_MASK "\n", "new", m_memory_stats_new.count, m_memory_stats_new.total_memory);
		write("%15s " MEMORY_INT_PRINT_MASK " " MEMORY_INT_PRINT_MASK "\n", "array", m_memory_stats_new_array.count, m_memory_stats_new_array.total_memory);
		write("%15s " MEMORY_INT_PRINT_MASK " " MEMORY_INT_PRINT_MASK "\n", "malloc", m_memory_stats_malloc.count, m_memory_stats_malloc.total_memory);

		write("\nProblems\n");
		write("----------------------------------------------------------------\n");

		resolverSetup();

		ProblemInfo* iterator = m_problems;
		while (iterator)
		{
			bool needs_allocation_stack = false;
			bool needs_deallocation_stack = false;

			write("{\n");
			write("[%d] %s\n", iterator->type, ProblemTypenames[ iterator->type ]);
			write("Count: %d\n", iterator->count);

			if (iterator->type == LeakType)
			{
				write("Total Memory Lost: %d \n", iterator->total_memory);
				needs_allocation_stack = true;
			}

			if ((iterator->type == RequestZeroMemoryType) ||
				(iterator->type == MismatchOperationType) ||
				(iterator->type == MemoryOverWriteType) ||
				(iterator->type == MultiAllocation))
			{
				needs_allocation_stack = true;
			}

			if ((iterator->type == MismatchOperationType) ||
				(iterator->type == ReleaseUnallocatedType) ||
				(iterator->type == MemoryOverWriteType))
			{
				needs_deallocation_stack = true;
			}

			if (needs_allocation_stack)
			{
				write("Allocation Stack: ");
				for (unsigned int frame = 0; frame != stack_size; ++frame)
				{
					if (iterator->allocation_stack[frame])
					{
						write("%p ", iterator->allocation_stack[frame]);
					}
				}
				write("\n");



				for (unsigned int frame = 0; frame != stack_size; ++frame)
				{
					if (iterator->allocation_stack[frame])
					{
						SourceCodeLocation location = resolveAddress((void*) (iterator->allocation_stack[frame]));
						if (location.line_number || location.filename || location.function)
						{
							write("[%d]\t%s:%d %s\n", frame, location.filename, location.line_number, location.function);
						}
					}
				}
			}


			if (needs_deallocation_stack)
			{
				write("Deallocation Stack: ");
				for (unsigned int frame = 0; frame != stack_size; ++frame)
				{
					if (iterator->deallocation_stack[frame])
					{
						write("%p ", iterator->deallocation_stack[frame]);
					}
				}
				write("\n");

				for (unsigned int frame = 0; frame != stack_size; ++frame)
				{
					if (iterator->deallocation_stack[frame])
					{
						SourceCodeLocation location = resolveAddress((void*) (iterator->deallocation_stack[frame]));
						if (location.line_number || location.filename || location.function)
						{
							write("[%d]\t%s:%d %s\n", frame, location.filename, location.line_number, location.function);
						}
					}
				}
			}


			write("}\n");
			iterator = iterator->next;
		}

		resolverTeardown();
	}

	void MemoryAllocator::onAllocate(void* pointer, SizeType size, OperationType type)
	{
		if (!size && ((type == NewType) || (type == NewArrayType) || (type == MallocType)) )
		{
			ConstPointer stack[stack_size];
			createStackTrace(stack);

			MemoryMutexScopedLock lock(*m_mutex);

			ProblemInfo* problem = findProblem(&m_problems, RequestZeroMemoryType, stack, m_null_stack);
			if (!problem)
			{
				problem = createProblem();
				problem->type = RequestZeroMemoryType;
				memcpy(problem->allocation_stack, stack, stack_size * sizeof(ConstPointer));
				addProblemToList(&m_problems, problem);
			}

			problem->count += 1;
		}
		else if (pointer)
		{
			MemoryMutexScopedLock lock(*m_mutex);

			OperationInfo* operation = findMemory(pointer);
			if (operation)
			{
				ConstPointer stack[stack_size];
				createStackTrace(stack);

				ProblemInfo* problem = findProblem(&m_problems, MultiAllocation, stack, m_null_stack);
				if (!problem)
				{
					problem = createProblem();
					problem->type = MultiAllocation;
					memcpy(problem->allocation_stack, stack, stack_size * sizeof(ConstPointer));
					addProblemToList(&m_problems, problem);
				}
			}
			else
			{
				OperationInfo* operation = createOperation();
				operation->memory_address = pointer;
				operation->type = type;
				operation->size = size;

				createStackTrace(operation->stack);
				addMemory(operation);

				m_memory_stats[type].count++;
				m_memory_stats[type].total_memory += size;

				if (MemoryInt(size) > m_memory_stats[type].max_value)
				{
					m_memory_stats[type].max_value = MemoryInt(size);
				}
			}
		}
	}

	MemoryAllocator::SizeType MemoryAllocator::memorySize(void* pointer)
	{
		SizeType size = 0;

		{
			MemoryMutexScopedLock lock(*m_mutex);
			OperationInfo* operation = findMemory(pointer);

			if (operation)
			{
				size = operation->size;
			}
		}

		return size;
	}

	void MemoryAllocator::onDeallocate(void* pointer, OperationType type)
	{
		if (pointer)
		{
			MemoryMutexScopedLock lock(*m_mutex);
			OperationInfo* operation = findMemory(pointer);

			if (operation)
			{
				m_memory_stats[type].count++;
				m_memory_stats[type].total_memory += operation->size;

				//
				// Check for mismatch operations
				//
				bool mismatched_release = true;
				if (type == DeleteType)
				{
					mismatched_release = (operation->type != NewType);
				}
				else if (type == DeleteArrayType)
				{
					mismatched_release = (operation->type != NewArrayType);
				}	
				else if (type == FreeType)
				{
					mismatched_release = (operation->type != MallocType);
				}

				if (mismatched_release)
				{
					ConstPointer stack[stack_size];
					createStackTrace(stack);

					ProblemInfo* problem = findProblem(&m_problems, MismatchOperationType, operation->stack, stack);
					if (!problem)
					{
						problem = createProblem();
						problem->type = MismatchOperationType;
						memcpy(problem->allocation_stack, operation->stack, stack_size * sizeof(ConstPointer));
						memcpy(problem->deallocation_stack, stack, stack_size * sizeof(ConstPointer));
						addProblemToList(&m_problems, problem);
					}
					problem->count += 1;
				}
			}
			else
			{
				//
				// Releasing unallocated data
				//
				ConstPointer stack[stack_size];
				createStackTrace(stack);

				ProblemInfo* problem = findProblem(&m_problems, ReleaseUnallocatedType, m_null_stack, stack);
				if (!problem)
				{
					problem = createProblem();
					problem->type = ReleaseUnallocatedType;
					memcpy(problem->deallocation_stack, stack, stack_size * sizeof(ConstPointer));
					addProblemToList(&m_problems, problem);
				}
				problem->count += 1;

				//
				// should we abort?
				//
			}

			removeMemory(operation);
		}
	}


	MemoryAllocator::OperationInfo* MemoryAllocator::createOperation()
	{
		OperationInfo* element = (OperationInfo*) malloc( sizeof(OperationInfo) );
		memset(element, 0, sizeof(OperationInfo) );
		return element;
	}

	MemoryAllocator::ProblemInfo* MemoryAllocator::createProblem()
	{
		ProblemInfo* element = (ProblemInfo*) malloc( sizeof(ProblemInfo) );
		memset(element, 0, sizeof(ProblemInfo) );
		return element;
	}

	bool MemoryAllocator::stacksMatch(ConstPointer lhs[stack_size], ConstPointer rhs[stack_size])
	{
		bool match = true;

		for (unsigned int i = 0; i != stack_size; ++i)
		{
			match &= (lhs[i] == rhs[i]);
		}

		return match;
	}

	void MemoryAllocator::emptyProblemList(ProblemInfo** list)
	{
		ProblemInfo* remove = 0;
		while (*list)
		{
			remove = *list;
			*list = (*list)->next;
			free(remove);
		}
	}

	void MemoryAllocator::addProblemToList(ProblemInfo** list, ProblemInfo* element)
	{
		element->next = *list;
		*list = element;
	}

	MemoryAllocator::ProblemInfo* MemoryAllocator::findProblem(ProblemInfo** list, ProblemType const type,
															   ConstPointer allocation_stack[stack_size],
															   ConstPointer deallocation_stack[stack_size])
	{
		ProblemInfo* iterator = *list;
		bool not_found = true;

		while(not_found && iterator)
		{
			not_found = ( (iterator->type != type)
						|| !stacksMatch(allocation_stack, iterator->allocation_stack)
						|| !stacksMatch(deallocation_stack, iterator->deallocation_stack) );

			if (not_found)
			{
				iterator = iterator->next;
			}
		}

		return iterator;
	}


	void MemoryAllocator::emptyOperationList(OperationInfo** list)
	{
		OperationInfo* remove = 0;
		while (*list)
		{
			remove = *list;
			*list = (*list)->next;
			free(remove);
		}
	}

	void MemoryAllocator::removeOperationFromList(OperationInfo** list, OperationInfo* element)
	{
		if (element)
		{
			OperationInfo* iterator = *list;

			if (iterator == element) // is the first element?
			{
				*list = element->next;
				free(element);
			}
			else
			{
				while (iterator && (iterator->next != element))
				{
					iterator = iterator->next;
				}

				if (iterator)
				{
					iterator->next = element->next;
					free(element);
				}
			}
		}
	}

	MemoryAllocator::OperationInfo* MemoryAllocator::findOperationOnList(OperationInfo* list, ConstPointer memory_address)
	{
		OperationInfo* iterator = list;
		while(iterator && (iterator->memory_address != memory_address) )
		{
			iterator = iterator->next;
		}
		return iterator;
	}

	void MemoryAllocator::addOperationToList(OperationInfo** list, OperationInfo* element)
	{
		element->next = *list;
		*list = element;
	}

	void MemoryAllocator::addMemory(OperationInfo* element)
	{
		if (element)
		{
			HashIterator hash_table_index = HASH_FUNCTION(element->memory_address);
			addOperationToList(&m_memory[hash_table_index], element);
		}
	}

	MemoryAllocator::OperationInfo* MemoryAllocator::findMemory(ConstPointer memory_address)
	{
		HashIterator hash_table_index = HASH_FUNCTION(memory_address);
		return findOperationOnList(m_memory[hash_table_index], memory_address);
	}

	void MemoryAllocator::removeMemory(OperationInfo* element)
	{
		if (element)
		{
			HashIterator hash_table_index = HASH_FUNCTION(element->memory_address);
			removeOperationFromList(&m_memory[hash_table_index], element);
		}
	}

	void MemoryAllocator::emptyMemory()
	{
		for (HashIterator i = 0; i != hash_table_size; ++i)
		{
			emptyOperationList(&m_memory[i]);
		}
	}

	void MemoryAllocator::enable(Bool enable_status)
	{
		m_enabled = enable_status;
	}

	bool MemoryAllocator::isEnabled() const
	{
		return m_enabled;
	}

	void* MemoryAllocator::allocate(SizeType size, OperationType type)
	{
		void* memory_pointer = malloc(size);

		if (isEnabled())
		{
			onAllocate(memory_pointer, size, type);
		}

		return memory_pointer;
	}

	void MemoryAllocator::deallocate(void* pointer, OperationType type)
	{
		if (isEnabled())
		{
			onDeallocate(pointer, type);
		}

		if (pointer)
		{
			free(pointer);
		}
	}

} // end of namespace

#if RENGINE_WITH_MEMORY_MANAGER == RENGINE_ON

#if RENGINE_COMPILER != RENGINE_COMPILER_MSVC
	void * operator new[] (size_t size) RENGINE_MEMORY_MANAGER_THROW
	{
		return rengine::MemoryAllocator::instance()->allocate(size, rengine::MemoryAllocator::NewArrayType);
	}

	void operator delete[] (void * p) RENGINE_MEMORY_MANAGER_THROW
	{
		rengine::MemoryAllocator::instance()->deallocate(p, rengine::MemoryAllocator::DeleteArrayType);
	}
#endif //RENGINE_COMPILER != RENGINE_COMPILER_MSVC

	void * operator new (size_t size) RENGINE_MEMORY_MANAGER_THROW
	{
		return rengine::MemoryAllocator::instance()->allocate(size, rengine::MemoryAllocator::NewType);
	}

	void operator delete (void * p) RENGINE_MEMORY_MANAGER_THROW
	{
		rengine::MemoryAllocator::instance()->deallocate(p, rengine::MemoryAllocator::DeleteType);
	}


	extern "C" 
	{
		void* rg_malloc(size_t size)
		{
			return rengine::MemoryAllocator::instance()->allocate(size, rengine::MemoryAllocator::MallocType);
		}

		void rg_free(void *p)
		{
			rengine::MemoryAllocator::instance()->deallocate(p, rengine::MemoryAllocator::FreeType);
		}

		void* rg_realloc(void *p, size_t size)
		{
			void* out = 0;

			if (size)
			{
				out = rg_malloc(size);

				if (out && p)
				{
					size_t current_size = rengine::MemoryAllocator::instance()->memorySize(p);

					if (size < current_size)
					{
						current_size = size;
					}

					memcpy(out, p, current_size);
				}
			} 
			
			if (p)
			{
				rg_free(p);
			}

			return out;
		}
	}

#else 
	void* rg_malloc(size_t size) { return malloc(size); }
	void rg_free(void *p) { free(p); }
	void* rg_realloc(void *p, size_t size) { return realloc(p, size); }

#endif //RENGINE_WITH_MEMORY_MANAGER

char * rg_strdup(char const* source)
{
	char* copy = 0;
	
	if (source)
	{
		size_t size = strlen(source);
		if (size >= 0)
		{
			copy = (char*) rg_malloc(size + 1);
			if (copy)
			{
				if (size)
				{
					memcpy(copy, source, size);
				}
				copy[size] = '\0';
			}
		}
	}

	return copy;
}

void* rg_calloc(size_t count, size_t size)
{
	void* out = 0;
	size *= count;

	if (size)
	{
		size *= count;
		out = rg_malloc(size);

		if (out)
		{
			memset(out, 0, size);
		}
	}

	return out;
}
