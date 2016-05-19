// __!!rengine_copyright!!__ //

#ifndef __RENGINE_MEMORY_ALLOCATOR_H__
#define __RENGINE_MEMORY_ALLOCATOR_H__

#include <rengine/lang/Types.h>

#include <new>

#include <cstdlib>
#include <cstdio>

namespace rengine
{
	class MemoryAllocator
	{
	public:
		struct MemoryMutex;

		static const unsigned int stack_size = 8;
		static const unsigned int operation_types = 6;
		static const unsigned int hash_table_size = 35323; //prime number, for many allocations use 343051

		typedef size_t SizeType;
		typedef SizeType HashIterator;

		typedef void* Pointer;
		typedef void const* ConstPointer;
		typedef Int64 MemoryInt;

		enum OperationType
		{
			NewType,
			DeleteType,
			NewArrayType,
			DeleteArrayType,
			MallocType,
			FreeType
		};

		enum ProblemType
		{
			LeakType,
			ReleaseUnallocatedType,
			MismatchOperationType,
			RequestZeroMemoryType,
			MemoryOverWriteType,
			MultiAllocation
		};

		struct MemoryStats
		{
			MemoryInt count;
			MemoryInt total_memory;
			MemoryInt max_value;
		};

		struct OperationInfo
		{
			OperationInfo *next;
			OperationType type;
			ConstPointer memory_address;
			SizeType size;
			ConstPointer stack[stack_size];
		};

		struct ProblemInfo
		{
			ProblemInfo *next;
			ProblemType type;
			size_t size;
			size_t count;
			MemoryInt total_memory;
			ConstPointer allocation_stack[stack_size];
			ConstPointer deallocation_stack[stack_size];
		};

		static MemoryAllocator* instance();

		void* allocate(SizeType size, OperationType type);
		void deallocate(void* pointer, OperationType type);
		SizeType memorySize(void* pointer);


		void enable(Bool enable_status = true);
		bool isEnabled() const;


		void analyzeMemory();
		void reportMemoryStatus();

	private:
		MemoryAllocator();
		~MemoryAllocator();

		//
		// Hash table management
		//
		void emptyMemory();
		void addMemory(OperationInfo* element);
		OperationInfo* findMemory(ConstPointer memory_address);
		void removeMemory(OperationInfo* element);


		//
		// Generic DCU_OperationInfo Linked-List Management
		//
		void emptyOperationList(OperationInfo** list);
		void removeOperationFromList(OperationInfo** list, OperationInfo* element);
		OperationInfo* findOperationOnList(OperationInfo* list, ConstPointer memory_address);
		void addOperationToList(OperationInfo** list, OperationInfo* element);

		//
		// Create
		//
		OperationInfo* createOperation();
		ProblemInfo* createProblem();

		bool stacksMatch(ConstPointer lhs[stack_size], ConstPointer rhs[stack_size]);


		//
		// Generic DCU_ProblemInfo Linked-List Management
		//
		void emptyProblemList(ProblemInfo** list);
		void addProblemToList(ProblemInfo** list, ProblemInfo* element);
		ProblemInfo* findProblem(ProblemInfo** list, ProblemType const type,
								 ConstPointer allocation_stack[stack_size],
								 ConstPointer deallocation_stack[stack_size]);


		void onAllocate(void* pointer, SizeType size, OperationType type);
		void onDeallocate(void* pointer, OperationType type);

		void write(char const* message, ...);

		volatile Bool m_enabled;

		OperationInfo** m_memory;
		ProblemInfo* m_problems;
		MemoryStats m_memory_stats[operation_types];
		MemoryStats m_memory_stats_new;
		MemoryStats m_memory_stats_new_array;
		MemoryStats m_memory_stats_malloc;
		ConstPointer m_null_stack[stack_size];

		FILE* m_output;

		MemoryMutex* m_mutex;
	};

} // end of namespace

#ifdef RENGINE_WITH_MEMORY_MANAGER

	#if RENGINE_COMPILER == RENGINE_COMPILER_GNUC
		#ifndef RENGINE_MEMORY_MANAGER_THROW
			#define RENGINE_MEMORY_MANAGER_THROW throw (std::bad_alloc)
		#endif //RENGINE_MEMORY_MANAGER_THROW
	#else 
		#define RENGINE_MEMORY_MANAGER_THROW
	#endif


	#if RENGINE_COMPILER != RENGINE_COMPILER_MSVC
		//Visual studio might call newaop.cpp new[] wich will call our new operator, this can cause incorrectly reported mismatch allocations
		void * operator new[] (size_t size) RENGINE_MEMORY_MANAGER_THROW;
		void operator delete[] (void * p) RENGINE_MEMORY_MANAGER_THROW;
	#endif //RENGINE_COMPILER != RENGINE_COMPILER_MSVC

	void * operator new (size_t size) RENGINE_MEMORY_MANAGER_THROW;
	void operator delete (void * p) RENGINE_MEMORY_MANAGER_THROW;

#endif //RENGINE_WITH_MEMORY_MANAGER


extern "C" 
{
	void* rg_malloc(size_t size);
	void rg_free(void *p);
	void* rg_realloc(void *p, size_t size);

	void* rg_calloc(size_t count, size_t size);
	char* rg_strdup(char const* source);
}

#endif // __RENGINE_MEMORY_ALLOCATOR_H__
