//----------------------------  thread_management.h  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2000, 2001, 2002 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  thread_management.h  ---------------------------
#ifndef __deal2__thread_management_h
#define __deal2__thread_management_h


#include <base/config.h>
#include <base/exceptions.h>

#include <utility>
#include <vector>
#include <iterator>

#ifdef DEAL_II_USE_MT
#  if defined(DEAL_II_USE_MT_ACE)
#    include <ace/Thread_Manager.h>
#    include <ace/Synch.h>
#  elif defined(DEAL_II_USE_MT_POSIX)
#    include <pthread.h>
#  endif
#endif




namespace Threads 
{
				   // forward declarations
  class FunDataBase;

/**
 * This class is used instead of a true lock class when not using
 * multithreading. It allows to write programs such that they start
 * new threads and/or lock objects in multithreading mode, and use
 * dummy thread management and synchronisation classes instead when
 * running in single-thread mode. Specifically, the @p{spawn} functions
 * only call the function but wait for it to return instead of running
 * in on another thread, and the mutices do nothing really. The only
 * reason to provide such a function is that the program can be
 * compiled both in MT and non-MT mode without difference.
 *
 * @author Wolfgang Bangerth, 2000
 */
  class DummyThreadMutex
  {
    public:
				       /**
					* Simulate acquisition of the
					* mutex. As this class does
					* nothing really, this
					* function does nothing as
					* well.
					*/
      inline void acquire () const {};

				       /**
					* Simulate release of the
					* mutex. As this class does
					* nothing really, this
					* function does nothing as
					* well.
					*/
      inline void release () const {};
  };


/**
 * This class is used instead of a true thread manager class when not
 * using multithreading. It allows to write programs such that they
 * start new threads and/or lock objects in multithreading mode, and
 * use dummy thread management and synchronisation classes instead
 * when running in single-thread mode. Specifically, the @p{spawn}
 * functions only call the function but wait for it to return instead
 * of running in on another thread, and the mutices do nothing
 * really. The only reason to provide such a function is that the
 * program can be compiled both in MT and non-MT mode without
 * difference.
 *
 * @author Wolfgang Bangerth, 2000
 */
  class DummyThreadManager 
  {
    public:
				       /**
					* Typedef for a global
					* function that might be
					* called on a new thread.
					*/
      typedef void * (*FunPtr) (void *);

				       /**
					* Emulate spawning a new
					* thread and calling the
					* passed function
					* thereon. Actually only call
					* that function with the given
					* parameters, and wait for it
					* to return.
					*/
      void spawn (const FunPtr fun_ptr,
		  void *       fun_data,
		  int          flags) const;

				       /**
					* Emulate that we wait for
					* other threads to
					* return. Since no other
					* threads have been started,
					* the right thing, of course,
					* is to return immediately.
					*/
      inline void wait () const {};
  };


  
/**
 * This class is used instead of a true barrier class when not using
 * multithreading. It allows to write programs such that they use the
 * same class names in multithreading and non-MT mode and thus may be
 * compiled with or without thread-support without the need to use
 * conditional compilation. Since a barrier class only makes sense in
 * non-multithread mode if only one thread is to be synchronised
 * (otherwise, the barrier could not be left, since the one thread is
 * waiting for some other part of the program to reach a certain point
 * of execution), the constructor of this class throws an exception if
 * the @p{count} argument denoting the number of threads that need to
 * be synchronised is not equal to one.
 *
 * @author Wolfgang Bangerth, 2001
 */
  class DummyBarrier
  {
    public:
				       /**
					* Constructor. Since barriers
					* are only useful in
					* single-threaded mode if the
					* number of threads to be
					* synchronised is one, this
					* constructor raises an
					* exception if the @p{count}
					* argument is one.
					*/
      DummyBarrier (const unsigned int  count,
		    const char         *name = 0,
		    void               *arg  = 0);

				       /**
					* Wait for all threads to
					* reach this point. Since
					* there may only be one
					* thread, return immediately,
					* i.e. this function is a
					* no-op.
					*/
      int wait () { return 0; };

				       /**
					* Dump the state of this
					* object. Here: do nothing.
					*/
      void dump () {};

				       /**
					* Exception.
					*/
      DeclException1 (ExcBarrierSizeNotUseful,
		      int,
		      << "In single-thread mode, other barrier sizes than 1 are not "
		      << "useful. You gave " << arg1);
  };
  
  
#ifdef DEAL_II_USE_MT
#  if defined(DEAL_II_USE_MT_ACE)
				   /**
				    * In multithread mode with ACE
				    * enabled, we alias the mutex and
				    * thread management classes to the
				    * respective classes of the ACE
				    * library. Likewise for the
				    * barrier class.
				    */
  typedef ACE_Thread_Mutex   ThreadMutex;
  typedef ACE_Thread_Manager ThreadManager;
  typedef ACE_Barrier        Barrier;
  
#  elif defined(DEAL_II_USE_MT_POSIX)

				   /**
				    * Class implementing a Mutex with
				    * the help of POSIX functions.
				    *
				    * @author Wolfgang Bangerth, 2002
				    */
  class PosixThreadMutex 
  {
    public:
				       /**
					* Constructor. Initialize the
					* underlying POSIX mutex data
					* structure.
					*/
      PosixThreadMutex ();

				       /**
					* Destructor. Release all
					* resources.
					*/
      ~PosixThreadMutex ();
      
				       /**
					* Acquire a mutex.
					*/
      inline void acquire () { pthread_mutex_lock(&mutex); };

				       /**
					* Release the mutex again.
					*/
      inline void release () { pthread_mutex_unlock(&mutex); };

    private:
				       /**
					* Data object storing the
					* POSIX data which we need to
					* call the POSIX functions.
					*/
      pthread_mutex_t mutex;
  };


				   /**
				    * Implementation of a thread
				    * barrier class, based on the
				    * POSIX thread functions. POSIX
				    * barriers are a relatively new
				    * feature and are not supported on
				    * all systems. If the
				    * configuration detected the
				    * absence of these functions, then
				    * barriers will not be available,
				    * and using this class will result
				    * in an exception been thrown. The
				    * rest of the threading
				    * functionality will be available,
				    * though.
				    *
				    * @author Wolfgang Bangerth, 2002
				    */
  class PosixThreadBarrier 
  {
    public:
				       /**
					* Constructor. Initialize the
					* underlying POSIX barrier data
					* structure.
					*/
      PosixThreadBarrier (const unsigned int  count,
			  const char         *name = 0,
			  void               *arg  = 0);

				       /**
					* Destructor. Release all
					* resources.
					*/
      ~PosixThreadBarrier ();      

				       /**
					* Wait for all threads to
					* reach this point. The return
					* value is zero for all
					* participating threads except
					* for one, for which the
					* return value is some
					* non-zero value. The
					* operating system picks the
					* special thread by some not
					* further known method.
					*/
      int wait ();

    private:
				       /**
					* Data object storing the
					* POSIX data which we need to
					* call the POSIX functions.
					*/
#ifndef DEAL_II_USE_MT_POSIX_NO_BARRIERS
      pthread_barrier_t barrier;
#endif
  };


  class PosixThreadManager 
  {
    public:
				       /**
					* Typedef for a global
					* function that might be
					* called on a new thread.
					*/
      typedef void * (*FunPtr) (void *);

				       /**
					* Constructor. Initialize data
					* structures.
					*/
      PosixThreadManager ();

				       /**
					* Destructor. Wait for all
					* spawned threads if they have
					* not yet finished, and
					* release the resources of
					* this object.
					*/
      ~PosixThreadManager ();
      
				       /**
					* Spawn a new thread and
					* calling the passed function
					* thereon. Store the
					* identifier of the thread for
					* later operations as waiting
					* for that thread.
					*
					* The @p{flags} argument is
					* currently ignored.
					*/
      void spawn (const FunPtr fun_ptr,
		  void *       fun_data,
		  int          flags);

				       /**
					* Wait for all spawned threads
					* to return.
					*/
      void wait () const;

    private:
				       /**
					* List of thread ids. This
					* variable actually points to
					* an object of type
					* @p{std::list<pthread_t>},
					* but to avoid including
					* @p{<list>} into this central
					* header file and all other
					* files including it, we use a
					* void pointer instead.
					*/
      void * const thread_id_list;
  };
  
  
  
  typedef PosixThreadMutex   ThreadMutex;
  typedef PosixThreadManager ThreadManager;
  typedef PosixThreadBarrier Barrier;
  
#  endif
#else
				   /**
				    * In non-multithread mode, the
				    * mutex and thread management
				    * classes are aliased to dummy
				    * classes that actually do
				    * nothing, in particular not lock
				    * objects or start new
				    * threads. Likewise for the
				    * barrier class.
				    */
  typedef DummyThreadMutex   ThreadMutex;
  typedef DummyThreadManager ThreadManager;
  typedef DummyBarrier       Barrier;
#endif



				   /**
				    * Given a class, argument type,
				    * and the return type, generate a
				    * local typedef denoting a pointer
				    * to such a member function.
				    */
  template <class Class, class RetType>
  struct MemFunPtr0
  {
      typedef RetType (Class::*type) ();
  };

#ifdef DEAL_II_CONST_MEM_FUN_PTR_BUG
				   /**
				    * Same as above, but for the case
				    * of a member function marked
				    * @p{const}. This should not
				    * really be necessary, but Intel's
				    * compiler has a bug here so we
				    * have to work around.
				    */
  template <class Class, class RetType>
  struct MemFunPtr0<const Class, RetType>
  {
      typedef RetType (Class::*type) () const;
  };
#endif  
				   /**
				    * Given a class, argument types,
				    * and the return type, generate a
				    * local typedef denoting a pointer
				    * to such a member function.
				    */
  template <class Class, class Arg1, class RetType>
  struct MemFunPtr1
  {
      typedef RetType (Class::*type) (Arg1);
  };

#ifdef DEAL_II_CONST_MEM_FUN_PTR_BUG
				   /**
				    * Same as above, but for the case
				    * of a member function marked
				    * @p{const}. This should not
				    * really be necessary, but Intel's
				    * compiler has a bug here so we
				    * have to work around.
				    */
  template <class Class, class Arg1, class RetType>
  struct MemFunPtr1<const Class, Arg1, RetType>
  {
      typedef RetType (Class::*type) (Arg1) const;
  };
#endif  
				   /**
				    * Given a class, argument types,
				    * and the return type, generate a
				    * local typedef denoting a pointer
				    * to such a member function.
				    */
  template <class Class, class Arg1, class Arg2, class RetType>
  struct MemFunPtr2
  {
      typedef RetType (Class::*type) (Arg1, Arg2);
  };

#ifdef DEAL_II_CONST_MEM_FUN_PTR_BUG
				   /**
				    * Same as above, but for the case
				    * of a member function marked
				    * @p{const}. This should not
				    * really be necessary, but Intel's
				    * compiler has a bug here so we
				    * have to work around.
				    */
  template <class Class, class Arg1, class Arg2, class RetType>
  struct MemFunPtr2<const Class, Arg1, Arg2, RetType>
  {
      typedef RetType (Class::*type) (Arg1, Arg2) const;
  };
#endif  

  				   /**
				    * Given a class, argument types,
				    * and the return type, generate a
				    * local typedef denoting a pointer
				    * to such a member function.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class RetType>
  struct MemFunPtr3
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3);
  };

#ifdef DEAL_II_CONST_MEM_FUN_PTR_BUG
				   /**
				    * Same as above, but for the case
				    * of a member function marked
				    * @p{const}. This should not
				    * really be necessary, but Intel's
				    * compiler has a bug here so we
				    * have to work around.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class RetType>
  struct MemFunPtr3<const Class, Arg1, Arg2, Arg3, RetType>
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3) const;
  };
#endif  

  				   /**
				    * Given a class, argument types,
				    * and the return type, generate a
				    * local typedef denoting a pointer
				    * to such a member function.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class RetType>
  struct MemFunPtr4
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4);
  };

#ifdef DEAL_II_CONST_MEM_FUN_PTR_BUG
				   /**
				    * Same as above, but for the case
				    * of a member function marked
				    * @p{const}. This should not
				    * really be necessary, but Intel's
				    * compiler has a bug here so we
				    * have to work around.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class RetType>
  struct MemFunPtr4<const Class, Arg1, Arg2, Arg3, Arg4, RetType>
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4) const;
  };
#endif  

  				   /**
				    * Given a class, argument types,
				    * and the return type, generate a
				    * local typedef denoting a pointer
				    * to such a member function.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class RetType>
  struct MemFunPtr5
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4, Arg5);
  };

#ifdef DEAL_II_CONST_MEM_FUN_PTR_BUG
				   /**
				    * Same as above, but for the case
				    * of a member function marked
				    * @p{const}. This should not
				    * really be necessary, but Intel's
				    * compiler has a bug here so we
				    * have to work around.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class RetType>
  struct MemFunPtr5<const Class, Arg1, Arg2, Arg3, Arg4, Arg5, RetType>
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4, Arg5) const;
  };
#endif  

  				   /**
				    * Given a class, argument types,
				    * and the return type, generate a
				    * local typedef denoting a pointer
				    * to such a member function.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class RetType>
  struct MemFunPtr6
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6);
  };

#ifdef DEAL_II_CONST_MEM_FUN_PTR_BUG
				   /**
				    * Same as above, but for the case
				    * of a member function marked
				    * @p{const}. This should not
				    * really be necessary, but Intel's
				    * compiler has a bug here so we
				    * have to work around.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class RetType>
  struct MemFunPtr6<const Class, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, RetType>
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const;
  };
#endif  

  				   /**
				    * Given a class, argument types,
				    * and the return type, generate a
				    * local typedef denoting a pointer
				    * to such a member function.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class RetType>
  struct MemFunPtr7
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7);
  };

#ifdef DEAL_II_CONST_MEM_FUN_PTR_BUG
				   /**
				    * Same as above, but for the case
				    * of a member function marked
				    * @p{const}. This should not
				    * really be necessary, but Intel's
				    * compiler has a bug here so we
				    * have to work around.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class RetType>
  struct MemFunPtr7<const Class, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, RetType>
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) const;
  };
#endif  


  				   /**
				    * Given a class, argument types,
				    * and the return type, generate a
				    * local typedef denoting a pointer
				    * to such a member function.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class RetType>
  struct MemFunPtr8
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8);
  };

#ifdef DEAL_II_CONST_MEM_FUN_PTR_BUG
				   /**
				    * Same as above, but for the case
				    * of a member function marked
				    * @p{const}. This should not
				    * really be necessary, but Intel's
				    * compiler has a bug here so we
				    * have to work around.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class RetType>
  struct MemFunPtr8<const Class, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, RetType>
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8) const;
  };
#endif  



  				   /**
				    * Given a class, argument types,
				    * and the return type, generate a
				    * local typedef denoting a pointer
				    * to such a member function.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Arg9, class RetType>
  struct MemFunPtr9
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9);
  };

#ifdef DEAL_II_CONST_MEM_FUN_PTR_BUG
				   /**
				    * Same as above, but for the case
				    * of a member function marked
				    * @p{const}. This should not
				    * really be necessary, but Intel's
				    * compiler has a bug here so we
				    * have to work around.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Arg9, class RetType>
  struct MemFunPtr9<const Class, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, RetType>
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9) const;
  };
#endif  

  


  				   /**
				    * Given a class, argument types,
				    * and the return type, generate a
				    * local typedef denoting a pointer
				    * to such a member function.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Arg9, class Arg10, class RetType>
  struct MemFunPtr10
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10);
  };

#ifdef DEAL_II_CONST_MEM_FUN_PTR_BUG
				   /**
				    * Same as above, but for the case
				    * of a member function marked
				    * @p{const}. This should not
				    * really be necessary, but Intel's
				    * compiler has a bug here so we
				    * have to work around.
				    */
  template <class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Arg9, class Arg10, class RetType>
  struct MemFunPtr10<const Class, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10, RetType>
  {
      typedef RetType (Class::*type) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10) const;
  };
#endif  

  
  
/**
 * Class used to store a pointer temporarily and delete the object
 * pointed to upon destruction of this object. For more information on
 * use and internals of this class see the report on multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  class FunEncapsulation
  {
    private:
				       /**
					* Default constructor. Construct
					* the object empty, i.e. set
					* @p{data==0}. Since this is not
					* very useful, disallow it by
					* declaring this constructor
					* @p{private}.
					*/
      FunEncapsulation ();

    public:
				       /**
					* Copy constructor. Clone the
					* object pointed to by
					* @p{fun_data.fun_data_base}.
					*/
      FunEncapsulation (const FunEncapsulation &fun_encapsulation);

				       /**
					* This is the usual
					* constructor. Set @p{fun_data_base} to
					* @p{fun_data_base}. This is what
					* the @p{fun_data_*} functions
					* use.
					*/
      FunEncapsulation (FunDataBase *fun_data_base);

				       /**
					* Destructor. Delete the object
					* pointed to by @p{fun_data_base}.
					*/
      ~FunEncapsulation ();

				       /**
					* Copy another object of this
					* type by cloning its @p{fun_data_base}
					* object.
					*/
      const FunEncapsulation & operator = (const FunEncapsulation &fun_encapsulation);
    
				       /**
					* Pointer to the object which
					* contains all the parameters.
					*/
      const FunDataBase * fun_data_base;
  };



/**
 * Abstract base class for those classes that actually store
 * parameters of functions. For more information on use and internals
 * of this class see the report on multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  class FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a
					* function that satifies the
					* requirements of thread entry
					* points.
					*/
      typedef void * (*ThreadEntryPoint) (void *);

				       /**
					* Default constructor. Store the
					* pointer to the function which
					* we will use as thread entry
					* point for the derived class.
					*/
      FunDataBase (const ThreadEntryPoint thread_entry_point);

				       /**
					* Copy constructor.
					*/
      FunDataBase (const FunDataBase &);

				       /**
					* Destructor. Needs to be
					* virtual to make destruction of
					* derived classes through base
					* class pointers.
					*/
      virtual ~FunDataBase ();

				       /**
					* Virtual constructor. Needed to
					* copy an object of which we
					* only have a pointer to the
					* base class. Copying such
					* objects is necessary to
					* guarantee memory consistency.
					*/
      virtual FunDataBase * clone () const = 0;

				       /**
					* Lock to be used when starting
					* a thread and which is released
					* after the data of this object
					* is copied and therefore no
					* more needed. This ensures that
					* no data is deleted when it is
					* still in use.
					*/
      mutable ThreadMutex lock;
    
    private:
				       /**
					* Pointer to the thread entry
					* point function. The address of
					* that function is passed from
					* the derived classes to the
					* constructor of this class.
					*/
      ThreadEntryPoint thread_entry_point;

				       /**
					* Make the thread starter
					* function a friend, since it
					* needs to have access to the
					* @p{thread_entry_point} variable.
					*/
      friend void spawn (ThreadManager          &thread_manager,
			 const FunEncapsulation &fun_data);
  };



/**
 * Class to store the parameters of a void function. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <typename RetType>
  class FunData0 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a global
					* function which we will call
					* from this class.
					*/
      typedef RetType (*FunPtr) ();

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      FunData0 (FunPtr fun_ptr);

				       /**
					* Copy constructor.
					*/
      FunData0 (const FunData0 &fun_data0);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);


//TODO:[WB] (compiler) with some compilers we get into trouble if the following class is not public, since we can't somehow declare the encapsulate function as friend. change this when compilers accept the friend declaration
  public:
				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename FunData0<RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    *
					    * Since the function in
					    * question here does not
					    * take parameters, this
					    * function also does
					    * nothing. It is only
					    * present for
					    * orthogonality of thread
					    * creation.
					    */
	  FunEncapsulation collect_args ();
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

// we should be declaring the following function friend, but gcc2.95
// does not allow us to do that at present...
//  				       /**
//  					* Declare a function that uses
//  					* the @ref{ArgCollector} as
//  					* friend.
//  					*/
//        friend
//        FunData0<void>::ArgCollector
//        encapsulate (void (*fun_ptr)());
  };



/**
 * Class to store the parameters of a unary function. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <typename Arg1, typename RetType>
  class FunData1 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a global
					* function which we will call
					* from this class.
					*/
      typedef RetType (*FunPtr) (Arg1);

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      FunData1 (FunPtr fun_ptr,
		Arg1   arg1);

				       /**
					* Copy constructor.
					*/
      FunData1 (const FunData1 &fun_data1);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;

				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename FunData1<Arg1,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Arg1 arg1);
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <typename Arg1_>
      friend
      typename FunData1<Arg1_,void>::ArgCollector
      encapsulate (void (*fun_ptr)(Arg1_));
  };



/**
 * Class to store the parameters of a binary function. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <typename Arg1, typename Arg2, typename RetType>
  class FunData2 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a global
					* function which we will call
					* from this class.
					*/
      typedef RetType (*FunPtr) (Arg1, Arg2);

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      FunData2 (FunPtr fun_ptr,
		Arg1   arg1,
		Arg2   arg2);

				       /**
					* Copy constructor.
					*/
      FunData2 (const FunData2 &fun_data2);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename FunData2<Arg1,Arg2,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Arg1 arg1,
					 Arg2 arg2);
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <typename Arg1_, typename Arg2_>
      friend
      typename FunData2<Arg1_,Arg2_,void>::ArgCollector
      encapsulate (void (*fun_ptr)(Arg1_, Arg2_));
  };

  
/**
 * Class to store the parameters of a ternary function. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <typename Arg1, typename Arg2, typename Arg3, typename RetType>
  class FunData3 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a global
					* function which we will call
					* from this class.
					*/
      typedef RetType (*FunPtr) (Arg1, Arg2, Arg3);

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      FunData3 (FunPtr fun_ptr,
		Arg1   arg1,
		Arg2   arg2,
		Arg3   arg3);

				       /**
					* Copy constructor.
					*/
      FunData3 (const FunData3 &fun_data3);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename FunData3<Arg1,Arg2,Arg3,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3);
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <typename Arg1_, typename Arg2_, typename Arg3_>
      friend
      typename FunData3<Arg1_,Arg2_,Arg3_,void>::ArgCollector
      encapsulate (void (*fun_ptr)(Arg1_,Arg2_,Arg3_));
  };


/**
 * Class to store the parameters of a quaternary function. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  class FunData4 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a global
					* function which we will call
					* from this class.
					*/
      typedef RetType (*FunPtr) (Arg1, Arg2, Arg3, Arg4);

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      FunData4 (FunPtr fun_ptr,
		Arg1   arg1,
		Arg2   arg2,
		Arg3   arg3,
		Arg4   arg4);

				       /**
					* Copy constructor.
					*/
      FunData4 (const FunData4 &fun_data4);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename FunData4<Arg1,Arg2,Arg3,Arg4,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4);
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_>
      friend
      typename FunData4<Arg1_,Arg2_,Arg3_,Arg4_,void>::ArgCollector
      encapsulate (void (*fun_ptr)(Arg1_, Arg2_, Arg3_, Arg4_));
  };



/**
 * Class to store the parameters of a quintary function. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  class FunData5 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a global
					* function which we will call
					* from this class.
					*/
      typedef RetType (*FunPtr) (Arg1, Arg2, Arg3, Arg4, Arg5);

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      FunData5 (FunPtr fun_ptr,
		Arg1   arg1,
		Arg2   arg2,
		Arg3   arg3,
		Arg4   arg4,
		Arg5   arg5);

				       /**
					* Copy constructor.
					*/
      FunData5 (const FunData5 &fun_data5);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      Arg5   arg5;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename FunData5<Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4,
					 Arg5 arg5);
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_>
      friend
      typename FunData5<Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,void>::ArgCollector
      encapsulate (void (*fun_ptr)(Arg1_, Arg2_, Arg3_, Arg4_, Arg5_));
  };


/**
 * Class to store the parameters of a function with 6 arguments. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  class FunData6 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a global
					* function which we will call
					* from this class.
					*/
      typedef RetType (*FunPtr) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6);

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      FunData6 (FunPtr fun_ptr,
		Arg1   arg1,
		Arg2   arg2,
		Arg3   arg3,
		Arg4   arg4,
		Arg5   arg5,
		Arg6   arg6);

				       /**
					* Copy constructor.
					*/
      FunData6 (const FunData6 &fun_data6);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      Arg5   arg5;
      Arg6   arg6;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename FunData6<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4,
					 Arg5 arg5,
					 Arg6 arg6);
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_>
      friend
      typename FunData6<Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,void>::ArgCollector
      encapsulate (void (*fun_ptr)(Arg1_, Arg2_, Arg3_,
				   Arg4_, Arg5_, Arg6_));
  };
  
  
/**
 * Class to store the parameters of a function with 7 arguments. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  class FunData7 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a global
					* function which we will call
					* from this class.
					*/
      typedef RetType (*FunPtr) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7);

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      FunData7 (FunPtr fun_ptr,
		Arg1   arg1,
		Arg2   arg2,
		Arg3   arg3,
		Arg4   arg4,
		Arg5   arg5,
		Arg6   arg6,
		Arg7   arg7);

				       /**
					* Copy constructor.
					*/
      FunData7 (const FunData7 &fun_data7);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      Arg5   arg5;
      Arg6   arg6;
      Arg7   arg7;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename FunData7<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4,
					 Arg5 arg5,
					 Arg6 arg6,
					 Arg7 arg7);
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_, typename Arg7_>
      friend
      typename FunData7<Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,Arg7_,void>::ArgCollector
      encapsulate (void (*fun_ptr)(Arg1_, Arg2_, Arg3_,
				   Arg4_, Arg5_, Arg6_, Arg7_));
  };
  
  
/**
 * Class to store the parameters of a function with 8 arguments. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  class FunData8 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a global
					* function which we will call
					* from this class.
					*/
      typedef RetType (*FunPtr) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8);

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      FunData8 (FunPtr fun_ptr,
		Arg1   arg1,
		Arg2   arg2,
		Arg3   arg3,
		Arg4   arg4,
		Arg5   arg5,
		Arg6   arg6,
		Arg7   arg7,
		Arg8   arg8);

				       /**
					* Copy constructor.
					*/
      FunData8 (const FunData8 &fun_data8);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      Arg5   arg5;
      Arg6   arg6;
      Arg7   arg7;
      Arg8   arg8;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename FunData8<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4,
					 Arg5 arg5,
					 Arg6 arg6,
					 Arg7 arg7,
					 Arg8 arg8);
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_, typename Arg7_, typename Arg8_>
      friend
      typename FunData8<Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,Arg7_,Arg8_,void>::ArgCollector
      encapsulate (void (*fun_ptr)(Arg1_, Arg2_, Arg3_,
				   Arg4_, Arg5_, Arg6_,
				   Arg7_, Arg8_));
  };
  
  
/**
 * Class to store the parameters of a function with 9 arguments. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2001
 */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  class FunData9 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a global
					* function which we will call
					* from this class.
					*/
      typedef RetType (*FunPtr) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9);

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      FunData9 (FunPtr fun_ptr,
		Arg1   arg1,
		Arg2   arg2,
		Arg3   arg3,
		Arg4   arg4,
		Arg5   arg5,
		Arg6   arg6,
		Arg7   arg7,
		Arg8   arg8,
		Arg9   arg9);

				       /**
					* Copy constructor.
					*/
      FunData9 (const FunData9 &fun_data9);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      Arg5   arg5;
      Arg6   arg6;
      Arg7   arg7;
      Arg8   arg8;
      Arg9   arg9;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename FunData9<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4,
					 Arg5 arg5,
					 Arg6 arg6,
					 Arg7 arg7,
					 Arg8 arg8,
					 Arg9 arg9);
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_, typename Arg7_, typename Arg8_, typename Arg9_>
      friend
      typename FunData9<Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,Arg7_,Arg8_,Arg9_,void>::ArgCollector
      encapsulate (void (*fun_ptr)(Arg1_, Arg2_, Arg3_,
				   Arg4_, Arg5_, Arg6_,
				   Arg7_, Arg8_, Arg9_));
  };
  
  
/**
 * Class to store the parameters of a function with 10 arguments. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2001
 */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  class FunData10 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a global
					* function which we will call
					* from this class.
					*/
      typedef RetType (*FunPtr) (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10);

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      FunData10 (FunPtr fun_ptr,
		 Arg1   arg1,
		 Arg2   arg2,
		 Arg3   arg3,
		 Arg4   arg4,
		 Arg5   arg5,
		 Arg6   arg6,
		 Arg7   arg7,
		 Arg8   arg8,
		 Arg9   arg9,
		 Arg10  arg10);

				       /**
					* Copy constructor.
					*/
      FunData10 (const FunData10 &fun_data10);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      Arg5   arg5;
      Arg6   arg6;
      Arg7   arg7;
      Arg8   arg8;
      Arg9   arg9;
      Arg10  arg10;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename FunData10<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4,
					 Arg5 arg5,
					 Arg6 arg6,
					 Arg7 arg7,
					 Arg8 arg8,
					 Arg9 arg9,
					 Arg10 arg10);
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_, typename Arg7_, typename Arg8_, typename Arg9_, typename Arg10_>
      friend
      typename FunData10<Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,Arg7_,Arg8_,Arg9_,Arg10_,void>::ArgCollector
      encapsulate (void (*fun_ptr)(Arg1_, Arg2_, Arg3_,
				   Arg4_, Arg5_, Arg6_,
				   Arg7_, Arg8_, Arg9_,
				   Arg10_));
  };
  
  
/**
 * Class to store the parameters of a void function. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <class Class, typename RetType>
  class MemFunData0 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a member
					* function which we will call
					* from this class.
					*/
      typedef typename MemFunPtr0<Class,RetType>::type FunPtr;

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      MemFunData0 (FunPtr fun_ptr,
		   Class *object);

				       /**
					* Copy constructor.
					*/
      MemFunData0 (const MemFunData0 &fun_data0);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Pointer to the object which
					* we shall work on.
					*/
      Class *object;
 
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename MemFunData0<Class,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Class *object);

					   /**
					    * Same as above, but take
					    * a reference instead of a
					    * pointer. This allows us
					    * to be a little
					    * convenient, as we can
					    * use @p{object} or
					    * @p{this}, without taking
					    * care that one is a
					    * reference and the other
					    * a pointer.
					    */
	  FunEncapsulation collect_args (Class &object);
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <class Class_>
      friend
      typename MemFunData0<Class_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)());

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
      template <class Class_>
      friend
      typename MemFunData0<const Class_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)() const);
#endif
  };



/**
 * Class to store the parameters of a unary function. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <class Class, typename Arg1, typename RetType>
  class MemFunData1 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a member
					* function which we will call
					* from this class.
					*/
      typedef typename MemFunPtr1<Class,Arg1,RetType>::type FunPtr;

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      MemFunData1 (FunPtr fun_ptr,
		   Class *object,
		   Arg1   arg1);

				       /**
					* Copy constructor.
					*/
      MemFunData1 (const MemFunData1 &fun_data1);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Pointer to the object which
					* we shall work on.
					*/
      Class *object;
      
				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;

				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename MemFunData1<Class,Arg1,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Class *object,
					 Arg1   arg1);

					   /**
					    * Same as above, but take
					    * a reference instead of a
					    * pointer. This allows us
					    * to be a little
					    * convenient, as we can
					    * use @p{object} or
					    * @p{this}, without taking
					    * care that one is a
					    * reference and the other
					    * a pointer.
					    */
	  FunEncapsulation collect_args (Class &object,
					 Arg1   arg1);

	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <class Class_, typename Arg1_>
      friend
      typename MemFunData1<Class_,Arg1_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
      template <class Class_, typename Arg1_>
      friend
      typename MemFunData1<const Class_,Arg1_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_) const);
#endif
  };



/**
 * Class to store the parameters of a binary function. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <class Class, typename Arg1, typename Arg2, typename RetType>
  class MemFunData2 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a member
					* function which we will call
					* from this class.
					*/
      typedef typename MemFunPtr2<Class,Arg1,Arg2,RetType>::type FunPtr;

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      MemFunData2 (FunPtr fun_ptr,
		   Class *object,
		   Arg1   arg1,
		   Arg2   arg2);

				       /**
					* Copy constructor.
					*/
      MemFunData2 (const MemFunData2 &fun_data2);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Pointer to the object which
					* we shall work on.
					*/
      Class *object;
 
				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename MemFunData2<Class,Arg1,Arg2,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Class *object,
					 Arg1 arg1,
					 Arg2 arg2);
	  
					   /**
					    * Same as above, but take
					    * a reference instead of a
					    * pointer. This allows us
					    * to be a little
					    * convenient, as we can
					    * use @p{object} or
					    * @p{this}, without taking
					    * care that one is a
					    * reference and the other
					    * a pointer.
					    */
	  FunEncapsulation collect_args (Class &object,
					 Arg1   arg1,
					 Arg2   arg2);
    
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <class Class_, typename Arg1_, typename Arg2_>
      friend
      typename MemFunData2<Class_,Arg1_,Arg2_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
      template <class Class_, typename Arg1_, typename Arg2_>
      friend
      typename MemFunData2<const Class_,Arg1_,Arg2_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_) const);
#endif
  };

  
/**
 * Class to store the parameters of a ternary function. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename RetType>
  class MemFunData3 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a member
					* function which we will call
					* from this class.
					*/
      typedef typename MemFunPtr3<Class,Arg1,Arg2,Arg3,RetType>::type FunPtr;

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      MemFunData3 (FunPtr fun_ptr,
		   Class *object,
		   Arg1   arg1,
		   Arg2   arg2,
		   Arg3   arg3);

				       /**
					* Copy constructor.
					*/
      MemFunData3 (const MemFunData3 &fun_data3);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Pointer to the object which
					* we shall work on.
					*/
      Class *object;
 
				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename MemFunData3<Class,Arg1,Arg2,Arg3,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Class *object,
					 Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3);
    
					   /**
					    * Same as above, but take
					    * a reference instead of a
					    * pointer. This allows us
					    * to be a little
					    * convenient, as we can
					    * use @p{object} or
					    * @p{this}, without taking
					    * care that one is a
					    * reference and the other
					    * a pointer.
					    */
	  FunEncapsulation collect_args (Class &object,
					 Arg1   arg1,
					 Arg2   arg2,
					 Arg3   arg3);

	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_>
      friend 
      typename MemFunData3<Class_,Arg1_,Arg2_,Arg3_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_,Arg2_,Arg3_));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_>
      friend 
      typename MemFunData3<const Class_,Arg1_,Arg2_,Arg3_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_,Arg2_,Arg3_) const);
#endif
  };



/**
 * Class to store the parameters of a quaternary function. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  class MemFunData4 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a member
					* function which we will call
					* from this class.
					*/
      typedef typename MemFunPtr4<Class,Arg1,Arg2,Arg3,Arg4,RetType>::type FunPtr;

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      MemFunData4 (FunPtr fun_ptr,
		   Class *object,
		   Arg1   arg1,
		   Arg2   arg2,
		   Arg3   arg3,
		   Arg4   arg4);

				       /**
					* Copy constructor.
					*/
      MemFunData4 (const MemFunData4 &fun_data4);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Pointer to the object which
					* we shall work on.
					*/
      Class *object;
 
				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Class *object,
					 Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4);
    
					   /**
					    * Same as above, but take
					    * a reference instead of a
					    * pointer. This allows us
					    * to be a little
					    * convenient, as we can
					    * use @p{object} or
					    * @p{this}, without taking
					    * care that one is a
					    * reference and the other
					    * a pointer.
					    */
	  FunEncapsulation collect_args (Class &object,
					 Arg1   arg1,
					 Arg2   arg2,
					 Arg3   arg3,
					 Arg4   arg4);
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_>
      friend
      typename MemFunData4<Class_,Arg1_,Arg2_,Arg3_,Arg4_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_, Arg4_));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_>
      friend
      typename MemFunData4<const Class_,Arg1_,Arg2_,Arg3_,Arg4_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_, Arg4_) const);
#endif  
};



/**
 * Class to store the parameters of a quintary function. For more
 * information on use and internals of this class see the report on
 * multithreading.
 *
 * @author Wolfgang Bangerth, 2000
 */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  class MemFunData5 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a member
					* function which we will call
					* from this class.
					*/
      typedef typename MemFunPtr5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::type FunPtr;

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      MemFunData5 (FunPtr fun_ptr,
		   Class *object,
		   Arg1   arg1,
		   Arg2   arg2,
		   Arg3   arg3,
		   Arg4   arg4,
		   Arg5   arg5);

				       /**
					* Copy constructor.
					*/
      MemFunData5 (const MemFunData5 &fun_data5);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Pointer to the object which
					* we shall work on.
					*/
      Class *object;
 
				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      Arg5   arg5;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Class *object,
					 Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4,
					 Arg5 arg5);
    
					   /**
					    * Same as above, but take
					    * a reference instead of a
					    * pointer. This allows us
					    * to be a little
					    * convenient, as we can
					    * use @p{object} or
					    * @p{this}, without taking
					    * care that one is a
					    * reference and the other
					    * a pointer.
					    */
	  FunEncapsulation collect_args (Class &object,
					 Arg1   arg1,
					 Arg2   arg2,
					 Arg3   arg3,
					 Arg4   arg4,
					 Arg5   arg5);
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_>
      friend 
      typename MemFunData5<Class_,Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_,
					   Arg4_, Arg5_));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_>
      friend 
      typename MemFunData5<const Class_,Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_,
					   Arg4_, Arg5_) const);
#endif
  };
  

  
/**
 * Class to store the parameters of a function with 6 arguments. For
 * more information on use and internals of this class see the report
 * on this subject.
 *
 * @author Wolfgang Bangerth, Ralf Hartmann, 2000
 */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  class MemFunData6 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a member
					* function which we will call
					* from this class.
					*/
      typedef typename MemFunPtr6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::type FunPtr;

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      MemFunData6 (FunPtr fun_ptr,
		   Class *object,
		   Arg1   arg1,
		   Arg2   arg2,
		   Arg3   arg3,
		   Arg4   arg4,
		   Arg5   arg5,
		   Arg6   arg6);

				       /**
					* Copy constructor.
					*/
      MemFunData6 (const MemFunData6 &fun_data6);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Pointer to the object which
					* we shall work on.
					*/
      Class *object;
 
				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      Arg5   arg5;
      Arg6   arg6;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Class *object,
					 Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4,
					 Arg5 arg5,
					 Arg6 arg6);
    
					   /**
					    * Same as above, but take
					    * a reference instead of a
					    * pointer. This allows us
					    * to be a little
					    * convenient, as we can
					    * use @p{object} or
					    * @p{this}, without taking
					    * care that one is a
					    * reference and the other
					    * a pointer.
					    */
	  FunEncapsulation collect_args (Class &object,
					 Arg1   arg1,
					 Arg2   arg2,
					 Arg3   arg3,
					 Arg4   arg4,
					 Arg5   arg5,
					 Arg6   arg6);
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_>
      friend 
      typename MemFunData6<Class_,Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_,
					   Arg4_, Arg5_, Arg6_));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_>
      friend 
      typename MemFunData6<const Class_,Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_,
					   Arg4_, Arg5_, Arg6_) const);
#endif  
  };



/**
 * Class to store the parameters of a function with 7 arguments. For
 * more information on use and internals of this class see the report
 * on this subject.
 *
 * @author Wolfgang Bangerth, Ralf Hartmann, 2001
 */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  class MemFunData7 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a member
					* function which we will call
					* from this class.
					*/
      typedef typename MemFunPtr7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::type FunPtr;

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      MemFunData7 (FunPtr fun_ptr,
		   Class *object,
		   Arg1   arg1,
		   Arg2   arg2,
		   Arg3   arg3,
		   Arg4   arg4,
		   Arg5   arg5,
		   Arg6   arg6,
		   Arg7   arg7);

				       /**
					* Copy constructor.
					*/
      MemFunData7 (const MemFunData7 &fun_data7);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Pointer to the object which
					* we shall work on.
					*/
      Class *object;
 
				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      Arg5   arg5;
      Arg6   arg6;
      Arg7   arg7;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Class *object,
					 Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4,
					 Arg5 arg5,
					 Arg6 arg6,
					 Arg7 arg7);
    
					   /**
					    * Same as above, but take
					    * a reference instead of a
					    * pointer. This allows us
					    * to be a little
					    * convenient, as we can
					    * use @p{object} or
					    * @p{this}, without taking
					    * care that one is a
					    * reference and the other
					    * a pointer.
					    */
	  FunEncapsulation collect_args (Class &object,
					 Arg1   arg1,
					 Arg2   arg2,
					 Arg3   arg3,
					 Arg4   arg4,
					 Arg5   arg5,
					 Arg6   arg6,
					 Arg7   arg7);
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_, typename Arg7_>
      friend 
      typename MemFunData7<Class_,Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,Arg7_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_,
					   Arg4_, Arg5_, Arg6_, Arg7_));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_, typename Arg7_>
      friend 
      typename MemFunData7<const Class_,Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,Arg7_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_,
					   Arg4_, Arg5_, Arg6_, Arg7_) const);
#endif
  };
  
  
/**
 * Class to store the parameters of a function with 8 arguments. For
 * more information on use and internals of this class see the report
 * on this subject.
 *
 * @author Wolfgang Bangerth, Ralf Hartmann, 2001
 */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  class MemFunData8 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a member
					* function which we will call
					* from this class.
					*/
      typedef typename MemFunPtr8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::type FunPtr;

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      MemFunData8 (FunPtr fun_ptr,
		   Class *object,
		   Arg1   arg1,
		   Arg2   arg2,
		   Arg3   arg3,
		   Arg4   arg4,
		   Arg5   arg5,
		   Arg6   arg6,
		   Arg7   arg7,
		   Arg8   arg8);

				       /**
					* Copy constructor.
					*/
      MemFunData8 (const MemFunData8 &fun_data8);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Pointer to the object which
					* we shall work on.
					*/
      Class *object;
 
				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      Arg5   arg5;
      Arg6   arg6;
      Arg7   arg7;
      Arg8   arg8;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Class *object,
					 Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4,
					 Arg5 arg5,
					 Arg6 arg6,
					 Arg7 arg7,
					 Arg8 arg8);
    
					   /**
					    * Same as above, but take
					    * a reference instead of a
					    * pointer. This allows us
					    * to be a little
					    * convenient, as we can
					    * use @p{object} or
					    * @p{this}, without taking
					    * care that one is a
					    * reference and the other
					    * a pointer.
					    */
	  FunEncapsulation collect_args (Class &object,
					 Arg1   arg1,
					 Arg2   arg2,
					 Arg3   arg3,
					 Arg4   arg4,
					 Arg5   arg5,
					 Arg6   arg6,
					 Arg7   arg7,
					 Arg8   arg8);
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_, typename Arg7_, typename Arg8_>
      friend 
      typename MemFunData8<Class_,Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,Arg7_,Arg8_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_,
					   Arg4_, Arg5_, Arg6_,
					   Arg7_, Arg8_));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_, typename Arg7_, typename Arg8_>
      friend 
      typename MemFunData8<const Class_,Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,Arg7_,Arg8_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_,
					   Arg4_, Arg5_, Arg6_,
					   Arg7_, Arg8_) const);
#endif
  };
  
  
/**
 * Class to store the parameters of a function with 9 arguments. For
 * more information on use and internals of this class see the report
 * on this subject.
 *
 * @author Wolfgang Bangerth, Ralf Hartmann, 2001
 */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  class MemFunData9 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a member
					* function which we will call
					* from this class.
					*/
      typedef typename MemFunPtr9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::type FunPtr;

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      MemFunData9 (FunPtr fun_ptr,
		   Class *object,
		   Arg1   arg1,
		   Arg2   arg2,
		   Arg3   arg3,
		   Arg4   arg4,
		   Arg5   arg5,
		   Arg6   arg6,
		   Arg7   arg7,
		   Arg8   arg8,
		   Arg9   arg9);

				       /**
					* Copy constructor.
					*/
      MemFunData9 (const MemFunData9 &fun_data9);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Pointer to the object which
					* we shall work on.
					*/
      Class *object;
 
				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      Arg5   arg5;
      Arg6   arg6;
      Arg7   arg7;
      Arg8   arg8;
      Arg9   arg9;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Class *object,
					 Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4,
					 Arg5 arg5,
					 Arg6 arg6,
					 Arg7 arg7,
					 Arg8 arg8,
					 Arg9 arg9);
    
					   /**
					    * Same as above, but take
					    * a reference instead of a
					    * pointer. This allows us
					    * to be a little
					    * convenient, as we can
					    * use @p{object} or
					    * @p{this}, without taking
					    * care that one is a
					    * reference and the other
					    * a pointer.
					    */
	  FunEncapsulation collect_args (Class &object,
					 Arg1   arg1,
					 Arg2   arg2,
					 Arg3   arg3,
					 Arg4   arg4,
					 Arg5   arg5,
					 Arg6   arg6,
					 Arg7   arg7,
					 Arg8   arg8,
					 Arg9   arg9);
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_, typename Arg7_, typename Arg8_, typename Arg9_>
      friend 
      typename MemFunData9<Class_,Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,Arg7_,Arg8_,Arg9_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_,
					   Arg4_, Arg5_, Arg6_,
					   Arg7_, Arg8_, Arg9_));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_, typename Arg7_, typename Arg8_, typename Arg9_>
      friend 
      typename MemFunData9<const Class_,Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,Arg7_,Arg8_,Arg9_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_,
					   Arg4_, Arg5_, Arg6_,
					   Arg7_, Arg8_, Arg9_) const);
#endif
  };
  
  
/**
 * Class to store the parameters of a function with 10 arguments. For
 * more information on use and internals of this class see the report
 * on this subject.
 *
 * @author Wolfgang Bangerth, Ralf Hartmann, 2001
 */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  class MemFunData10 : public FunDataBase
  {
    public:
				       /**
					* Typedef a pointer to a member
					* function which we will call
					* from this class.
					*/
      typedef typename MemFunPtr10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::type FunPtr;

				       /**
					* Constructor. Store pointer to
					* the function and the values of
					* the arguments.
					*/
      MemFunData10 (FunPtr fun_ptr,
		    Class *object,
		    Arg1   arg1,
		    Arg2   arg2,
		    Arg3   arg3,
		    Arg4   arg4,
		    Arg5   arg5,
		    Arg6   arg6,
		    Arg7   arg7,
		    Arg8   arg8,
		    Arg9   arg9,
		    Arg10  arg10);

				       /**
					* Copy constructor.
					*/
      MemFunData10 (const MemFunData10 &fun_data10);

				       /**
					* Virtual constructor.
					*/
      virtual FunDataBase * clone () const;

    private:

				       /**
					* Pointer to the function to be
					* called and values of the
					* arguments to be passed.
					*/
      FunPtr fun_ptr;

				       /**
					* Pointer to the object which
					* we shall work on.
					*/
      Class *object;
 
				       /**
					* Values of the arguments of the
					* function to be called.
					*/
      Arg1   arg1;
      Arg2   arg2;
      Arg3   arg3;
      Arg4   arg4;
      Arg5   arg5;
      Arg6   arg6;
      Arg7   arg7;
      Arg8   arg8;
      Arg9   arg9;
      Arg10  arg10;
      
				       /**
					* Static function used as entry
					* point for the new thread.
					*/
      static void * thread_entry_point (void *arg);

				       /**
					* Helper class, used to collect
					* the values of the parameters
					* which we will pass to the
					* function, once we know its
					* type.
					*/
      class ArgCollector
      {
	public:
					   /**
					    * Typedef the function
					    * pointer type of the
					    * underlying class to a
					    * local type.
					    */
	  typedef typename MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::FunPtr FunPtr;
	
					   /**
					    * Constructor. Take and store a
					    * pointer to the function which
					    * is to be called.
					    */
	  ArgCollector (FunPtr fun_ptr);
    
					   /**
					    * Take the arguments with
					    * which we want to call the
					    * function and produce and
					    * object of the desired type
					    * from that.
					    */
	  FunEncapsulation collect_args (Class *object,
					 Arg1 arg1,
					 Arg2 arg2,
					 Arg3 arg3,
					 Arg4 arg4,
					 Arg5 arg5,
					 Arg6 arg6,
					 Arg7 arg7,
					 Arg8 arg8,
					 Arg9 arg9,
					 Arg10 arg10);
    
					   /**
					    * Same as above, but take
					    * a reference instead of a
					    * pointer. This allows us
					    * to be a little
					    * convenient, as we can
					    * use @p{object} or
					    * @p{this}, without taking
					    * care that one is a
					    * reference and the other
					    * a pointer.
					    */
	  FunEncapsulation collect_args (Class &object,
					 Arg1   arg1,
					 Arg2   arg2,
					 Arg3   arg3,
					 Arg4   arg4,
					 Arg5   arg5,
					 Arg6   arg6,
					 Arg7   arg7,
					 Arg8   arg8,
					 Arg9   arg9,
					 Arg10  arg10);
	private:
					   /**
					    * Space to temporarily store
					    * the function pointer.
					    */
	  FunPtr fun_ptr;
      };

				       /**
					* Declare a function that uses
					* the @ref{ArgCollector} as
					* friend.
					*/
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_, typename Arg7_, typename Arg8_, typename Arg9_, typename Arg10_>
      friend 
      typename MemFunData10<Class_,Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,Arg7_,Arg8_,Arg9_,Arg10_,void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_,
					   Arg4_, Arg5_, Arg6_,
					   Arg7_, Arg8_, Arg9_, Arg10_));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
      template <class Class_, typename Arg1_, typename Arg2_, typename Arg3_, typename Arg4_, typename Arg5_, typename Arg6_, typename Arg7_, typename Arg8_, typename Arg9_, typename Arg10_>
      friend 
      typename MemFunData10<const Class_,Arg1_,Arg2_,Arg3_,Arg4_,Arg5_,Arg6_,Arg7_,Arg8_,Arg9_,Arg10_void>::ArgCollector
      encapsulate (void (Class_::*fun_ptr)(Arg1_, Arg2_, Arg3_,
					   Arg4_, Arg5_, Arg6_,
					   Arg7_, Arg8_, Arg9_, Arg10_) const);
#endif
  };
  
  

				   /**
				    * Encapsulate a function pointer
				    * into an object with which a new
				    * thread can later be spawned.
				    * For more information on use and
				    * internals of this class see the
				    * report on multithreading.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  inline
  FunData0<void>::ArgCollector
  encapsulate (void (*fun_ptr)());

				   /**
				    * Encapsulate a function pointer
				    * into an object with which a new
				    * thread can later be spawned.
				    * For more information on use and
				    * internals of this class see the
				    * report on multithreading.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <typename Arg1>
  inline
  typename FunData1<Arg1,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1));


				   /**
				    * Encapsulate a function pointer
				    * into an object with which a new
				    * thread can later be spawned.
				    * For more information on use and
				    * internals of this class see the
				    * report on multithreading.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <typename Arg1, typename Arg2>
  inline
  typename FunData2<Arg1,Arg2,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2));
  

				   /**
				    * Encapsulate a function pointer
				    * into an object with which a new
				    * thread can later be spawned.
				    * For more information on use and
				    * internals of this class see the
				    * report on multithreading.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <typename Arg1, typename Arg2, typename Arg3>
  inline
  typename FunData3<Arg1,Arg2,Arg3,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3));
  

				   /**
				    * Encapsulate a function pointer
				    * into an object with which a new
				    * thread can later be spawned.
				    * For more information on use and
				    * internals of this class see the
				    * report on multithreading.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
  inline
  typename FunData4<Arg1,Arg2,Arg3,Arg4,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4));



				   /**
				    * Encapsulate a function pointer
				    * into an object with which a new
				    * thread can later be spawned.
				    * For more information on use and
				    * internals of this class see the
				    * report on multithreading.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
  inline
  typename FunData5<Arg1,Arg2,Arg3,Arg4,Arg5,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5));


				   /**
				    * Encapsulate a function pointer
				    * into an object with which a new
				    * thread can later be spawned.
				    * For more information on use and
				    * internals of this class see the
				    * report on multithreading.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
  inline
  typename FunData6<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6));


				   /**
				    * Encapsulate a function pointer
				    * into an object with which a new
				    * thread can later be spawned.
				    * For more information on use and
				    * internals of this class see the
				    * report on multithreading.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
  inline
  typename FunData7<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7));


				   /**
				    * Encapsulate a function pointer
				    * into an object with which a new
				    * thread can later be spawned.
				    * For more information on use and
				    * internals of this class see the
				    * report on multithreading.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
  inline
  typename FunData8<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8));


				   /**
				    * Encapsulate a function pointer
				    * into an object with which a new
				    * thread can later be spawned.
				    * For more information on use and
				    * internals of this class see the
				    * report on multithreading.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
  inline
  typename FunData9<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9));


				   /**
				    * Encapsulate a function pointer
				    * into an object with which a new
				    * thread can later be spawned.
				    * For more information on use and
				    * internals of this class see the
				    * report on multithreading.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
  inline
  typename FunData10<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10));


				   /**
				    * Encapsulate a member function
				    * pointer into an object with
				    * which a new thread can later be
				    * spawned.  For more information
				    * on use and internals of this
				    * class see the report on this
				    * subject.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <class Class>
  inline
  typename MemFunData0<Class,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)());

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
                                   /**
				    * Same as the previous function,
				    * but for member functions marked
				    * @p{const}. This function should
				    * not be necessary, since the
				    * compiler should deduce a
				    * constant class as template
				    * argument, but we have to work
				    * around a bug in Intel's icc
				    * compiler with this.  
                                    */
  template <class Class>
  inline
  typename MemFunData0<const Class,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)() const);
#endif
 
				   /**
				    * Encapsulate a member function
				    * pointer into an object with
				    * which a new thread can later be
				    * spawned.  For more information
				    * on use and internals of this
				    * class see the report on this
				    * subject.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <class Class, typename Arg1>
  inline
  typename MemFunData1<Class,Arg1,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
                                   /**
				    * Same as the previous function,
				    * but for member functions marked
				    * @p{const}. This function should
				    * not be necessary, since the
				    * compiler should deduce a
				    * constant class as template
				    * argument, but we have to work
				    * around a bug in Intel's icc
				    * compiler with this.  
                                    */
  template <class Class, typename Arg1>
  inline
  typename MemFunData1<const Class,Arg1,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1) const);
#endif
 

				   /**
				    * Encapsulate a member function
				    * pointer into an object with
				    * which a new thread can later be
				    * spawned.  For more information
				    * on use and internals of this
				    * class see the report on this
				    * subject.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <class Class, typename Arg1, typename Arg2>
  inline
  typename MemFunData2<Class,Arg1,Arg2,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
                                   /**
				    * Same as the previous function,
				    * but for member functions marked
				    * @p{const}. This function should
				    * not be necessary, since the
				    * compiler should deduce a
				    * constant class as template
				    * argument, but we have to work
				    * around a bug in Intel's icc
				    * compiler with this.  
                                    */
  template <class Class, typename Arg1, typename Arg2>
  inline
  typename MemFunData2<const Class,Arg1,Arg2,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2) const);
#endif
   

				   /**
				    * Encapsulate a member function
				    * pointer into an object with
				    * which a new thread can later be
				    * spawned.  For more information
				    * on use and internals of this
				    * class see the report on this
				    * subject.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3>
  inline
  typename MemFunData3<Class,Arg1,Arg2,Arg3,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
                                   /**
				    * Same as the previous function,
				    * but for member functions marked
				    * @p{const}. This function should
				    * not be necessary, since the
				    * compiler should deduce a
				    * constant class as template
				    * argument, but we have to work
				    * around a bug in Intel's icc
				    * compiler with this.  
                                    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3>
  inline
  typename MemFunData3<const Class,Arg1,Arg2,Arg3,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3) const);
#endif
   

				   /**
				    * Encapsulate a member function
				    * pointer into an object with
				    * which a new thread can later be
				    * spawned.  For more information
				    * on use and internals of this
				    * class see the report on this
				    * subject.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
  inline
  typename MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
                                   /**
				    * Same as the previous function,
				    * but for member functions marked
				    * @p{const}. This function should
				    * not be necessary, since the
				    * compiler should deduce a
				    * constant class as template
				    * argument, but we have to work
				    * around a bug in Intel's icc
				    * compiler with this.  
                                    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
  inline
  typename MemFunData4<const Class,Arg1,Arg2,Arg3,Arg4,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4) const);
#endif
   

				   /**
				    * Encapsulate a member function
				    * pointer into an object with
				    * which a new thread can later be
				    * spawned.  For more information
				    * on use and internals of this
				    * class see the report on this
				    * subject.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
  inline
  typename MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
                                   /**
				    * Same as the previous function,
				    * but for member functions marked
				    * @p{const}. This function should
				    * not be necessary, since the
				    * compiler should deduce a
				    * constant class as template
				    * argument, but we have to work
				    * around a bug in Intel's icc
				    * compiler with this.  
                                    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
  inline
  typename MemFunData5<const Class,Arg1,Arg2,Arg3,Arg4,Arg5,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5) const);
#endif
 

				   /**
				    * Encapsulate a member function
				    * pointer into an object with
				    * which a new thread can later be
				    * spawned.  For more information
				    * on use and internals of this
				    * class see the report on this
				    * subject.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
  inline
  typename MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
                                   /**
				    * Same as the previous function,
				    * but for member functions marked
				    * @p{const}. This function should
				    * not be necessary, since the
				    * compiler should deduce a
				    * constant class as template
				    * argument, but we have to work
				    * around a bug in Intel's icc
				    * compiler with this.  
                                    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
  inline
  typename MemFunData6<const Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const);
#endif
 
				   /**
				    * Encapsulate a member function
				    * pointer into an object with
				    * which a new thread can later be
				    * spawned.  For more information
				    * on use and internals of this
				    * class see the report on this
				    * subject.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
  inline
  typename MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
                                   /**
				    * Same as the previous function,
				    * but for member functions marked
				    * @p{const}. This function should
				    * not be necessary, since the
				    * compiler should deduce a
				    * constant class as template
				    * argument, but we have to work
				    * around a bug in Intel's icc
				    * compiler with this.  
                                    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
  inline
  typename MemFunData7<const Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) const);
#endif
 
				   /**
				    * Encapsulate a member function
				    * pointer into an object with
				    * which a new thread can later be
				    * spawned.  For more information
				    * on use and internals of this
				    * class see the report on this
				    * subject.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
  inline
  typename MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
                                   /**
				    * Same as the previous function,
				    * but for member functions marked
				    * @p{const}. This function should
				    * not be necessary, since the
				    * compiler should deduce a
				    * constant class as template
				    * argument, but we have to work
				    * around a bug in Intel's icc
				    * compiler with this.  
                                    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
  inline
  typename MemFunData8<const Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8) const);
#endif
 
				   /**
				    * Encapsulate a member function
				    * pointer into an object with
				    * which a new thread can later be
				    * spawned.  For more information
				    * on use and internals of this
				    * class see the report on this
				    * subject.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
  inline
  typename MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
                                   /**
				    * Same as the previous function,
				    * but for member functions marked
				    * @p{const}. This function should
				    * not be necessary, since the
				    * compiler should deduce a
				    * constant class as template
				    * argument, but we have to work
				    * around a bug in Intel's icc
				    * compiler with this.  
                                    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
  inline
  typename MemFunData9<const Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9) const);
#endif
 
				   /**
				    * Encapsulate a member function
				    * pointer into an object with
				    * which a new thread can later be
				    * spawned.  For more information
				    * on use and internals of this
				    * class see the report on this
				    * subject.
				    *
				    * This function exists once for
				    * each number of parameters.
				    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
  inline
  typename MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10));

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
                                   /**
				    * Same as the previous function,
				    * but for member functions marked
				    * @p{const}. This function should
				    * not be necessary, since the
				    * compiler should deduce a
				    * constant class as template
				    * argument, but we have to work
				    * around a bug in Intel's icc
				    * compiler with this.  
                                    */
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
  inline
  typename MemFunData10<const Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10) const);
#endif
 
  
				   /**
				    * Spawn a new thread using the
				    * function and parameters
				    * encapsulated in @p{fun_data}, and
				    * using the given thread manager
				    * object.
				    */
  void spawn (ThreadManager          &thread_manager,
	      const FunEncapsulation &fun_data);


				   /**
				    * Spawn several threads at once,
				    * using the same parameters and
				    * calling the same function.
				    */
  void spawn_n (ThreadManager          &thread_manager,
		const FunEncapsulation &fun_encapsulation,
		const unsigned int      n_threads);


				   /**
				    * Split the range @p{[begin,end)}
				    * into @p{n_intervals} subintervals
				    * of equal size. The last interval
				    * will be a little bit larger, if
				    * the number of elements in the
				    * whole range is not exactly
				    * divisible by @p{n_intervals}. The
				    * type of the iterators has to
				    * fulfill the requirements of a
				    * forward iterator,
				    * i.e. @p{operator++} must be
				    * available, and of course it must
				    * be assignable.
				    *
				    * A list of subintervals is
				    * returned as a vector of pairs of
				    * iterators, where each pair
				    * denotes the range
				    * @p{[begin[i],end[i])}.
				    */
  template <typename ForwardIterator>
  typename std::vector<typename std::pair<ForwardIterator,ForwardIterator> >
  split_range (const ForwardIterator &begin,
	       const ForwardIterator &end,
	       const unsigned int n_intervals);

				   /**
				    * Split the interval @p{[begin,end)}
				    * into subintervals of (almost)
				    * equal size. This function works
				    * mostly as the one before, with
				    * the difference that instead of
				    * iterators, now values are taken
				    * that define the whole interval.
				    */
  std::vector<std::pair<unsigned int,unsigned int> >
  split_interval (const unsigned int begin,
		  const unsigned int end,
		  const unsigned int n_intervals);
  
  

/**
 * This class is used to make some sanity checks on the numbers of
 * objects of some types related with thread spawning, which are
 * created and deleted. This is a helpful thing when trying to
 * implement the data copying using @p{clone} functions etc, in order to
 * avoid that there are some objects which are copied but not deleted.
 *
 * It basically only monitors the number of objects which is alive at
 * each time, and complains if the number is nonzero when the counting
 * object is deleted. Since one will probably want to use one global
 * counter, the complaint is raised at the end of the program, and
 * then means that somewhen within the lifetime of your program there
 * has occured a memory leak.
 *
 * This class is not meant for public use.
 *
 * @author Wolfgang Bangerth, 2000
 */
  struct FunDataCounter
  {
				       /**
					* Constructor. Sets all
					* counters to zero.
					*/
      FunDataCounter ();
      
				       /**
					* Destructor. Check whether
					* the total number of objects
					* is zero, otherwise throw an
					* exception.
					*/
      ~FunDataCounter ();

				       /**
					* Counters for the two types
					* of objects which we
					* presently monitor.
					*/
      unsigned int n_fun_encapsulation_objects;
      unsigned int n_fun_data_base_objects;

				       /**
					* Exception
					*/
      DeclException2 (ExcObjectsExist,
		      char*, int,
		      << "There are still " << arg2 << " objects of type "
		      << arg1 << " alive. You probably have a memory "
		      << "leak somewhere.");
  };

};   // end declarations of namespace Threads







/* ----------- implementation of functions in namespace Threads ---------- */
namespace Threads 
{

/* ---------------------- FunData0 implementation ------------------------ */

  template <typename RetType>
  FunData0<RetType>::FunData0 (FunPtr fun_ptr) :
		  FunDataBase (&FunData0<RetType>::thread_entry_point),
		  fun_ptr (fun_ptr)
  {}



  template <typename RetType>
  FunData0<RetType>::FunData0 (const FunData0 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr)
  {}



  template <typename RetType>
  FunDataBase *
  FunData0<RetType>::clone () const 
  {
    return new FunData0 (*this);
  }



  template <typename RetType>
  void *
  FunData0<RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef FunData0<RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (*fun_ptr)();
  
    return 0;
  }



  template <typename RetType>
  FunData0<RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}
 	

  
  template <typename RetType>
  FunEncapsulation
  FunData0<RetType>::ArgCollector::collect_args ()
  {
    return new FunData0<void>(fun_ptr);
  }
 


/* ---------------------- FunData1 implementation ------------------------ */

  template <typename Arg1, typename RetType>
  FunData1<Arg1,RetType>::FunData1 (FunPtr fun_ptr,
				    Arg1   arg1) :
		  FunDataBase (&FunData1<Arg1,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  arg1 (arg1)
  {}



  template <typename Arg1, typename RetType>
  FunData1<Arg1,RetType>::FunData1 (const FunData1 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  arg1 (fun_data.arg1)
  {}



  template <typename Arg1, typename RetType>
  FunDataBase *
  FunData1<Arg1,RetType>::clone () const 
  {
    return new FunData1 (*this);
  }



  template <typename Arg1, typename RetType>
  void *
  FunData1<Arg1,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef FunData1<Arg1,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Arg1              arg1    = fun_data->arg1;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (*fun_ptr)(arg1);
  
    return 0;
  }



  template <typename Arg1, typename RetType>
  FunData1<Arg1,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}
 	

  template <typename Arg1, typename RetType>
  FunEncapsulation
  FunData1<Arg1,RetType>::ArgCollector::collect_args (Arg1 arg1)
  {
    return new FunData1<Arg1,void>(fun_ptr, arg1);
  }
 
  


/* ---------------------- FunData2 implementation ------------------------ */

  template <typename Arg1, typename Arg2, typename RetType>
  FunData2<Arg1,Arg2,RetType>::FunData2 (FunPtr fun_ptr,
					 Arg1   arg1,
					 Arg2   arg2) :
		  FunDataBase (&FunData2<Arg1,Arg2,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  arg1 (arg1),
		  arg2 (arg2)
  {}



  template <typename Arg1, typename Arg2, typename RetType>
  FunData2<Arg1,Arg2,RetType>::FunData2 (const FunData2 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2)
  {}



  template <typename Arg1, typename Arg2, typename RetType>
  FunDataBase *
  FunData2<Arg1,Arg2,RetType>::clone () const 
  {
    return new FunData2 (*this);
  }



  template <typename Arg1, typename Arg2, typename RetType>
  void *
  FunData2<Arg1,Arg2,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef FunData2<Arg1,Arg2,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (*fun_ptr)(arg1, arg2);
  
    return 0;
  }



  template <typename Arg1, typename Arg2, typename RetType>
  FunData2<Arg1,Arg2,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}
 	

  
  template <typename Arg1, typename Arg2, typename RetType>
  FunEncapsulation
  FunData2<Arg1,Arg2,RetType>::ArgCollector::collect_args (Arg1 arg1,
							   Arg2 arg2)
  {
    return new FunData2<Arg1,Arg2,void>(fun_ptr, arg1, arg2);
  }
 
  


/* ---------------------- FunData3 implementation ------------------------ */

  template <typename Arg1, typename Arg2, typename Arg3, typename RetType>
  FunData3<Arg1,Arg2,Arg3,RetType>::FunData3 (FunPtr fun_ptr,
					      Arg1   arg1,
					      Arg2   arg2,
					      Arg3   arg3) :
		  FunDataBase (&FunData3<Arg1,Arg2,Arg3,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename RetType>
  FunData3<Arg1,Arg2,Arg3,RetType>::FunData3 (const FunData3 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename RetType>
  FunDataBase *
  FunData3<Arg1,Arg2,Arg3,RetType>::clone () const 
  {
    return new FunData3 (*this);
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename RetType>
  void *
  FunData3<Arg1,Arg2,Arg3,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef FunData3<Arg1,Arg2,Arg3,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (*fun_ptr)(arg1, arg2, arg3);
  
    return 0;
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename RetType>
  FunData3<Arg1,Arg2,Arg3,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}
 	

  
  template <typename Arg1, typename Arg2, typename Arg3, typename RetType>
  FunEncapsulation
  FunData3<Arg1,Arg2,Arg3,RetType>::ArgCollector::collect_args (Arg1 arg1,
								Arg2 arg2,
								Arg3 arg3)
  {
    return new FunData3<Arg1,Arg2,Arg3,void>(fun_ptr, arg1, arg2, arg3);
  }
 
  


/* ---------------------- FunData4 implementation ------------------------ */

  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  FunData4<Arg1,Arg2,Arg3,Arg4,RetType>::FunData4 (FunPtr fun_ptr,
						   Arg1   arg1,
						   Arg2   arg2,
						   Arg3   arg3,
						   Arg4   arg4) :
		  FunDataBase (&FunData4<Arg1,Arg2,Arg3,Arg4,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  FunData4<Arg1,Arg2,Arg3,Arg4,RetType>::FunData4 (const FunData4 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  FunDataBase *
  FunData4<Arg1,Arg2,Arg3,Arg4,RetType>::clone () const 
  {
    return new FunData4 (*this);
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  void *
  FunData4<Arg1,Arg2,Arg3,Arg4,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef FunData4<Arg1,Arg2,Arg3,Arg4,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (*fun_ptr)(arg1, arg2, arg3, arg4);
  
    return 0;
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  FunData4<Arg1,Arg2,Arg3,Arg4,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  FunEncapsulation
  FunData4<Arg1,Arg2,Arg3,Arg4,RetType>::ArgCollector::collect_args (Arg1 arg1,
								     Arg2 arg2,
								     Arg3 arg3,
								     Arg4 arg4)
  {
    return new FunData4<Arg1,Arg2,Arg3,Arg4,void>(fun_ptr, arg1, arg2, arg3, arg4);
  }
 
  


/* ---------------------- FunData5 implementation ------------------------ */

  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  FunData5<Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::FunData5 (FunPtr fun_ptr,
							Arg1   arg1,
							Arg2   arg2,
							Arg3   arg3,
							Arg4   arg4,
							Arg5   arg5) :
		  FunDataBase (&FunData5<Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4),
		  arg5 (arg5)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  FunData5<Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::FunData5 (const FunData5 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4),
		  arg5 (fun_data.arg5)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  FunDataBase *
  FunData5<Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::clone () const 
  {
    return new FunData5 (*this);
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  void *
  FunData5<Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef FunData5<Arg1,Arg2,Arg3,Arg4,Arg5,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;
    Arg5              arg5    = fun_data->arg5;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (*fun_ptr)(arg1, arg2, arg3, arg4, arg5);
  
    return 0;
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  FunData5<Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  FunEncapsulation
  FunData5<Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::ArgCollector::collect_args (Arg1 arg1,
									  Arg2 arg2,
									  Arg3 arg3,
									  Arg4 arg4,
									  Arg5 arg5)
  {
    return new FunData5<Arg1,Arg2,Arg3,Arg4,Arg5,void>(fun_ptr, arg1, arg2,
						       arg3, arg4, arg5);
  }



  
/* ---------------------- FunData6 implementation ------------------------ */

  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  FunData6<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::FunData6 (FunPtr fun_ptr,
							     Arg1   arg1,
							     Arg2   arg2,
							     Arg3   arg3,
							     Arg4   arg4,
							     Arg5   arg5,
							     Arg6   arg6) :
		  FunDataBase (&FunData6<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4),
		  arg5 (arg5),
		  arg6 (arg6)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  FunData6<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::FunData6 (const FunData6 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4),
		  arg5 (fun_data.arg5),
		  arg6 (fun_data.arg6)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  FunDataBase *
  FunData6<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::clone () const 
  {
    return new FunData6 (*this);
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  void *
  FunData6<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef FunData6<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;
    Arg5              arg5    = fun_data->arg5;
    Arg6              arg6    = fun_data->arg6;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (*fun_ptr)(arg1, arg2, arg3, arg4, arg5, arg6);
  
    return 0;
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  FunData6<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  FunEncapsulation
  FunData6<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::ArgCollector::collect_args (Arg1 arg1,
									       Arg2 arg2,
									       Arg3 arg3,
									       Arg4 arg4,
									       Arg5 arg5,
									       Arg6 arg6)
  {
    return new FunData6<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,void>(fun_ptr, arg1, arg2,
							    arg3, arg4, arg5, arg6);
  }
 
  



/* ---------------------- FunData7 implementation ------------------------ */

  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  FunData7<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::FunData7 (FunPtr fun_ptr,
								  Arg1   arg1,
								  Arg2   arg2,
								  Arg3   arg3,
								  Arg4   arg4,
								  Arg5   arg5,
								  Arg6   arg6,
								  Arg7   arg7) :
		  FunDataBase (&FunData7<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4),
		  arg5 (arg5),
		  arg6 (arg6),
		  arg7 (arg7)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  FunData7<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::FunData7 (const FunData7 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4),
		  arg5 (fun_data.arg5),
		  arg6 (fun_data.arg6),
		  arg7 (fun_data.arg7)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  FunDataBase *
  FunData7<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::clone () const 
  {
    return new FunData7 (*this);
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  void *
  FunData7<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef FunData7<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;
    Arg5              arg5    = fun_data->arg5;
    Arg6              arg6    = fun_data->arg6;
    Arg7              arg7    = fun_data->arg7;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (*fun_ptr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
  
    return 0;
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  FunData7<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  FunEncapsulation
  FunData7<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::ArgCollector::collect_args (Arg1 arg1,
										    Arg2 arg2,
										    Arg3 arg3,
										    Arg4 arg4,
										    Arg5 arg5,
										    Arg6 arg6,
										    Arg7 arg7)
  {
    return new FunData7<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,void>(fun_ptr, arg1, arg2,
								 arg3, arg4, arg5, arg6, arg7);
  }
 
  



/* ---------------------- FunData8 implementation ------------------------ */

  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  FunData8<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::
  FunData8 (FunPtr fun_ptr,
	    Arg1   arg1,
	    Arg2   arg2,
	    Arg3   arg3,
	    Arg4   arg4,
	    Arg5   arg5,
	    Arg6   arg6,
	    Arg7   arg7,
	    Arg8   arg8) :
		  FunDataBase (&FunData8<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4),
		  arg5 (arg5),
		  arg6 (arg6),
		  arg7 (arg7),
		  arg8 (arg8)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  FunData8<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::
  FunData8 (const FunData8 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4),
		  arg5 (fun_data.arg5),
		  arg6 (fun_data.arg6),
		  arg7 (fun_data.arg7),
		  arg8 (fun_data.arg8)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  FunDataBase *
  FunData8<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::clone () const 
  {
    return new FunData8 (*this);
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  void *
  FunData8<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef FunData8<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;
    Arg5              arg5    = fun_data->arg5;
    Arg6              arg6    = fun_data->arg6;
    Arg7              arg7    = fun_data->arg7;
    Arg8              arg8    = fun_data->arg8;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (*fun_ptr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
  
    return 0;
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  FunData8<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  FunEncapsulation
  FunData8<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::ArgCollector::collect_args (Arg1 arg1,
											 Arg2 arg2,
											 Arg3 arg3,
											 Arg4 arg4,
											 Arg5 arg5,
											 Arg6 arg6,
											 Arg7 arg7,
											 Arg8 arg8)
  {
    return new FunData8<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,void>(fun_ptr, arg1, arg2,
								      arg3, arg4, arg5, arg6,
								      arg7, arg8);
  }
 
  

/* ---------------------- FunData9 implementation ------------------------ */

  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  FunData9<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::
  FunData9 (FunPtr fun_ptr,
	    Arg1   arg1,
	    Arg2   arg2,
	    Arg3   arg3,
	    Arg4   arg4,
	    Arg5   arg5,
	    Arg6   arg6,
	    Arg7   arg7,
	    Arg8   arg8,
	    Arg9   arg9) :
		  FunDataBase (&FunData9<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4),
		  arg5 (arg5),
		  arg6 (arg6),
		  arg7 (arg7),
		  arg8 (arg8),
		  arg9 (arg9)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  FunData9<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::
  FunData9 (const FunData9 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4),
		  arg5 (fun_data.arg5),
		  arg6 (fun_data.arg6),
		  arg7 (fun_data.arg7),
		  arg8 (fun_data.arg8),
		  arg9 (fun_data.arg9)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  FunDataBase *
  FunData9<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::clone () const 
  {
    return new FunData9 (*this);
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  void *
  FunData9<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef FunData9<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;
    Arg5              arg5    = fun_data->arg5;
    Arg6              arg6    = fun_data->arg6;
    Arg7              arg7    = fun_data->arg7;
    Arg8              arg8    = fun_data->arg8;
    Arg9              arg9    = fun_data->arg9;    


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (*fun_ptr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
  
    return 0;
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  FunData9<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  FunEncapsulation
  FunData9<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::ArgCollector::
  collect_args (Arg1 arg1,
		Arg2 arg2,
		Arg3 arg3,
		Arg4 arg4,
		Arg5 arg5,
		Arg6 arg6,
		Arg7 arg7,
		Arg8 arg8,
		Arg9 arg9)
  {
    return
      new FunData9<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,void>
      (fun_ptr, arg1, arg2,
       arg3, arg4, arg5, arg6,
       arg7, arg8, arg9);
  }
 
  

/* ---------------------- FunData10 implementation ------------------------ */

  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  FunData10<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::
  FunData10 (FunPtr fun_ptr,
	     Arg1   arg1,
	     Arg2   arg2,
	     Arg3   arg3,
	     Arg4   arg4,
	     Arg5   arg5,
	     Arg6   arg6,
	     Arg7   arg7,
	     Arg8   arg8,
	     Arg9   arg9,
	     Arg10  arg10) :
		  FunDataBase (&FunData10<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4),
		  arg5 (arg5),
		  arg6 (arg6),
		  arg7 (arg7),
		  arg8 (arg8),
		  arg9 (arg9),
		  arg10 (arg10)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  FunData10<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::
  FunData10 (const FunData10 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4),
		  arg5 (fun_data.arg5),
		  arg6 (fun_data.arg6),
		  arg7 (fun_data.arg7),
		  arg8 (fun_data.arg8),
		  arg9 (fun_data.arg9),
		  arg10 (fun_data.arg10)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  FunDataBase *
  FunData10<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::clone () const 
  {
    return new FunData10 (*this);
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  void *
  FunData10<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef FunData10<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;
    Arg5              arg5    = fun_data->arg5;
    Arg6              arg6    = fun_data->arg6;
    Arg7              arg7    = fun_data->arg7;
    Arg8              arg8    = fun_data->arg8;
    Arg9              arg9    = fun_data->arg9;
    Arg10             arg10   = fun_data->arg10;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (*fun_ptr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
  
    return 0;
  }



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  FunData10<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  FunEncapsulation
  FunData10<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::ArgCollector::
  collect_args (Arg1 arg1,
		Arg2 arg2,
		Arg3 arg3,
		Arg4 arg4,
		Arg5 arg5,
		Arg6 arg6,
		Arg7 arg7,
		Arg8 arg8,
		Arg9 arg9,
		Arg10 arg10)
  {
    return
      new FunData10<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,void>
      (fun_ptr, arg1, arg2,
       arg3, arg4, arg5, arg6,
       arg7, arg8, arg9, arg10);
  }
 
  



/* ---------------------- MemFunData0 implementation ------------------------ */

  template <class Class, typename RetType>
  MemFunData0<Class,RetType>::MemFunData0 (FunPtr  fun_ptr,
					   Class  *object) :
		  FunDataBase (&MemFunData0<Class,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  object (object)
  {}



  template <class Class, typename RetType>
  MemFunData0<Class,RetType>::MemFunData0 (const MemFunData0 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  object (fun_data.object)
  {}



  template <class Class, typename RetType>
  FunDataBase *
  MemFunData0<Class,RetType>::clone () const 
  {
    return new MemFunData0 (*this);
  }



  template <class Class, typename RetType>
  void *
  MemFunData0<Class,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef MemFunData0<Class,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Class            *object  = fun_data->object;

				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (object->*fun_ptr)();
  
    return 0;
  }



  template <class Class, typename RetType>
  MemFunData0<Class,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}
 	

  
  template <class Class, typename RetType>
  FunEncapsulation
  MemFunData0<Class,RetType>::ArgCollector::collect_args (Class *object)
  {
    return new MemFunData0<Class,void>(fun_ptr, object);
  }
 


  template <class Class, typename RetType>
  inline
  FunEncapsulation
  MemFunData0<Class,RetType>::ArgCollector::collect_args (Class &object)
  {
    return collect_args (&object);
  }
 


/* ---------------------- MemFunData1 implementation ------------------------ */

  template <class Class, typename Arg1, typename RetType>
  MemFunData1<Class,Arg1,RetType>::MemFunData1 (FunPtr  fun_ptr,
						Class  *object,
						Arg1    arg1) :
		  FunDataBase (&MemFunData1<Class,Arg1,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  object (object),
		  arg1 (arg1)
  {}



  template <class Class, typename Arg1, typename RetType>
  MemFunData1<Class,Arg1,RetType>::MemFunData1 (const MemFunData1 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  object (fun_data.object),
		  arg1 (fun_data.arg1)
  {}



  template <class Class, typename Arg1, typename RetType>
  FunDataBase *
  MemFunData1<Class,Arg1,RetType>::clone () const 
  {
    return new MemFunData1 (*this);
  }



  template <class Class, typename Arg1, typename RetType>
  void *
  MemFunData1<Class,Arg1,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef MemFunData1<Class,Arg1,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Class            *object  = fun_data->object;
    Arg1              arg1    = fun_data->arg1;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (object->*fun_ptr)(arg1);
  
    return 0;
  }



  template <class Class, typename Arg1, typename RetType>
  MemFunData1<Class,Arg1,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}
 	

  template <class Class, typename Arg1, typename RetType>
  FunEncapsulation
  MemFunData1<Class,Arg1,RetType>::ArgCollector::collect_args (Class *object,
							       Arg1   arg1)
  {
    return new MemFunData1<Class,Arg1,void>(fun_ptr, object, arg1);
  }
 
  

  template <class Class, typename Arg1, typename RetType>
  inline
  FunEncapsulation
  MemFunData1<Class,Arg1,RetType>::ArgCollector::collect_args (Class &object,
							       Arg1   arg1)
  {
    return collect_args (&object, arg1);
  }

  
/* ---------------------- MemFunData2 implementation ------------------------ */

  template <class Class, typename Arg1, typename Arg2, typename RetType>
  MemFunData2<Class,Arg1,Arg2,RetType>::MemFunData2 (FunPtr  fun_ptr,
						     Class  *object,
						     Arg1    arg1,
						     Arg2    arg2) :
		  FunDataBase (&MemFunData2<Class,Arg1,Arg2,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  object (object),
		  arg1 (arg1),
		  arg2 (arg2)
  {}



  template <class Class, typename Arg1, typename Arg2, typename RetType>
  MemFunData2<Class,Arg1,Arg2,RetType>::MemFunData2 (const MemFunData2 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  object (fun_data.object),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2)
  {}



  template <class Class, typename Arg1, typename Arg2, typename RetType>
  FunDataBase *
  MemFunData2<Class,Arg1,Arg2,RetType>::clone () const 
  {
    return new MemFunData2 (*this);
  }



  template <class Class, typename Arg1, typename Arg2, typename RetType>
  void *
  MemFunData2<Class,Arg1,Arg2,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef MemFunData2<Class,Arg1,Arg2,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Class            *object  = fun_data->object;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (object->*fun_ptr)(arg1, arg2);
  
    return 0;
  }



  template <class Class, typename Arg1, typename Arg2, typename RetType>
  MemFunData2<Class,Arg1,Arg2,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}
 	

  
  template <class Class, typename Arg1, typename Arg2, typename RetType>
  FunEncapsulation
  MemFunData2<Class,Arg1,Arg2,RetType>::ArgCollector::collect_args (Class *object,
								    Arg1   arg1,
								    Arg2   arg2)
  {
    return new MemFunData2<Class,Arg1,Arg2,void>(fun_ptr, object, arg1, arg2);
  }
 
  
  template <class Class, typename Arg1, typename Arg2, typename RetType>
  inline
  FunEncapsulation
  MemFunData2<Class,Arg1,Arg2,RetType>::ArgCollector::collect_args (Class &object,
								    Arg1   arg1,
								    Arg2   arg2)
  {
    return collect_args (&object, arg1, arg2);
  }
 
  


/* ---------------------- MemFunData3 implementation ------------------------ */

  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename RetType>
  MemFunData3<Class,Arg1,Arg2,Arg3,RetType>::MemFunData3 (FunPtr  fun_ptr,
							  Class  *object,
							  Arg1    arg1,
							  Arg2    arg2,
							  Arg3    arg3) :
		  FunDataBase (&MemFunData3<Class,Arg1,Arg2,Arg3,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  object (object),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename RetType>
  MemFunData3<Class,Arg1,Arg2,Arg3,RetType>::MemFunData3 (const MemFunData3 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  object (fun_data.object),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename RetType>
  FunDataBase *
  MemFunData3<Class,Arg1,Arg2,Arg3,RetType>::clone () const 
  {
    return new MemFunData3 (*this);
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename RetType>
  void *
  MemFunData3<Class,Arg1,Arg2,Arg3,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef MemFunData3<Class,Arg1,Arg2,Arg3,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Class            *object  = fun_data->object;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (object->*fun_ptr)(arg1, arg2, arg3);
  
    return 0;
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename RetType>
  MemFunData3<Class,Arg1,Arg2,Arg3,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}
 	

  
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename RetType>
  FunEncapsulation
  MemFunData3<Class,Arg1,Arg2,Arg3,RetType>::ArgCollector::collect_args (Class *object,
									 Arg1   arg1,
									 Arg2   arg2,
									 Arg3   arg3)
  {
    return new MemFunData3<Class,Arg1,Arg2,Arg3,void>(fun_ptr, object,
						      arg1, arg2, arg3);
  }
 
  
  
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename RetType>
  inline
  FunEncapsulation
  MemFunData3<Class,Arg1,Arg2,Arg3,RetType>::ArgCollector::collect_args (Class &object,
									 Arg1   arg1,
									 Arg2   arg2,
									 Arg3   arg3)
  {
    return collect_args (&object, arg1, arg2, arg3);
  }
  


/* ---------------------- MemFunData4 implementation ------------------------ */

  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,RetType>::MemFunData4 (FunPtr  fun_ptr,
							       Class  *object,
							       Arg1    arg1,
							       Arg2    arg2,
							       Arg3    arg3,
							       Arg4    arg4) :
		  FunDataBase (&MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  object (object),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,RetType>::MemFunData4 (const MemFunData4 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  object (fun_data.object),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  FunDataBase *
  MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,RetType>::clone () const 
  {
    return new MemFunData4 (*this);
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  void *
  MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Class            *object  = fun_data->object;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (object->*fun_ptr)(arg1, arg2, arg3, arg4);
  
    return 0;
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  FunEncapsulation
  MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,RetType>::ArgCollector::collect_args (Class *object,
									      Arg1   arg1,
									      Arg2   arg2,
									      Arg3   arg3,
									      Arg4   arg4)
  {
    return new MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,void>(fun_ptr, object,
							   arg1, arg2, arg3, arg4);
  }
 
  

  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename RetType>
  inline
  FunEncapsulation
  MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,RetType>::ArgCollector::collect_args (Class &object,
									      Arg1   arg1,
									      Arg2   arg2,
									      Arg3   arg3,
									      Arg4   arg4)
  {
    return collect_args (&object, arg1, arg2, arg3, arg4);
  }
  

/* ---------------------- MemFunData5 implementation ------------------------ */

  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::MemFunData5 (FunPtr  fun_ptr,
								    Class  *object,
								    Arg1    arg1,
								    Arg2    arg2,
								    Arg3    arg3,
								    Arg4    arg4,
								    Arg5    arg5) :
		  FunDataBase (&MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  object (object),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4),
		  arg5 (arg5)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::MemFunData5 (const MemFunData5 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  object (fun_data.object),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4),
		  arg5 (fun_data.arg5)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  FunDataBase *
  MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::clone () const 
  {
    return new MemFunData5 (*this);
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  void *
  MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Class            *object  = fun_data->object;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;
    Arg5              arg5    = fun_data->arg5;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (object->*fun_ptr)(arg1, arg2, arg3, arg4, arg5);
  
    return 0;
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  FunEncapsulation
  MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::ArgCollector::collect_args (Class *object,
										   Arg1   arg1,
										   Arg2   arg2,
										   Arg3   arg3,
										   Arg4   arg4,
										   Arg5   arg5)
  {
    return new MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,void>(fun_ptr, object,
								arg1, arg2, arg3, arg4, arg5);
  }
 

  
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename RetType>
  inline
  FunEncapsulation
  MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,RetType>::ArgCollector::collect_args (Class &object,
										   Arg1   arg1,
										   Arg2   arg2,
										   Arg3   arg3,
										   Arg4   arg4,
										   Arg5   arg5)
  {
    return collect_args (&object, arg1, arg2, arg3, arg4, arg5);
  }
  

  
/* ---------------------- MemFunData6 implementation ------------------------ */

  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::MemFunData6 (FunPtr  fun_ptr,
									 Class  *object,
									 Arg1    arg1,
									 Arg2    arg2,
									 Arg3    arg3,
									 Arg4    arg4,
									 Arg5    arg5,
									 Arg6    arg6) :
		  FunDataBase (&MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  object (object),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4),
		  arg5 (arg5),
		  arg6 (arg6)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::MemFunData6 (const MemFunData6 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  object (fun_data.object),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4),
		  arg5 (fun_data.arg5),
		  arg6 (fun_data.arg6)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  FunDataBase *
  MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::clone () const 
  {
    return new MemFunData6 (*this);
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  void *
  MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Class            *object  = fun_data->object;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;
    Arg5              arg5    = fun_data->arg5;
    Arg6              arg6    = fun_data->arg6;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (object->*fun_ptr)(arg1, arg2, arg3, arg4, arg5, arg6);
  
    return 0;
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  FunEncapsulation
  MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::ArgCollector::collect_args (Class *object,
											Arg1   arg1,
											Arg2   arg2,
											Arg3   arg3,
											Arg4   arg4,
											Arg5   arg5,
											Arg6   arg6)
  {
    return new MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,void>(fun_ptr, object,
								     arg1, arg2, arg3, arg4, arg5, arg6);
  }
  


  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename RetType>
  inline
  FunEncapsulation
  MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,RetType>::ArgCollector::collect_args (Class &object,
											Arg1   arg1,
											Arg2   arg2,
											Arg3   arg3,
											Arg4   arg4,
											Arg5   arg5,
											Arg6   arg6)
  {
    return collect_args (&object, arg1, arg2, arg3, arg4, arg5, arg6);
  }



  

/* ---------------------- MemFunData7 implementation ------------------------ */

  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::MemFunData7 (FunPtr  fun_ptr,
									      Class  *object,
									      Arg1    arg1,
									      Arg2    arg2,
									      Arg3    arg3,
									      Arg4    arg4,
									      Arg5    arg5,
									      Arg6    arg6,
									      Arg7    arg7) :
		  FunDataBase (&MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  object (object),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4),
		  arg5 (arg5),
		  arg6 (arg6),
		  arg7 (arg7)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::MemFunData7 (const MemFunData7 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  object (fun_data.object),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4),
		  arg5 (fun_data.arg5),
		  arg6 (fun_data.arg6),
		  arg7 (fun_data.arg7)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  FunDataBase *
  MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::clone () const 
  {
    return new MemFunData7 (*this);
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  void *
  MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Class            *object  = fun_data->object;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;
    Arg5              arg5    = fun_data->arg5;
    Arg6              arg6    = fun_data->arg6;
    Arg7              arg7    = fun_data->arg7;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (object->*fun_ptr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
  
    return 0;
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  FunEncapsulation
  MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::ArgCollector::collect_args (Class *object,
											     Arg1   arg1,
											     Arg2   arg2,
											     Arg3   arg3,
											     Arg4   arg4,
											     Arg5   arg5,
											     Arg6   arg6,
											     Arg7   arg7)
  {
    return new MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,void>(fun_ptr, object,
									  arg1, arg2, arg3, arg4, arg5, arg6, arg7);
  }
  


  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename RetType>
  inline
  FunEncapsulation
  MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,RetType>::ArgCollector::collect_args (Class &object,
											     Arg1   arg1,
											     Arg2   arg2,
											     Arg3   arg3,
											     Arg4   arg4,
											     Arg5   arg5,
											     Arg6   arg6,
											     Arg7   arg7)
  {
    return collect_args (&object, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
  }
  

/* ---------------------- MemFunData8 implementation ------------------------ */
  
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::MemFunData8 (FunPtr  fun_ptr,
										   Class  *object,
										   Arg1    arg1,
										   Arg2    arg2,
										   Arg3    arg3,
										   Arg4    arg4,
										   Arg5    arg5,
										   Arg6    arg6,
										   Arg7    arg7,
										   Arg8    arg8) :
		  FunDataBase (&MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  object (object),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4),
		  arg5 (arg5),
		  arg6 (arg6),
		  arg7 (arg7),
		  arg8 (arg8)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::MemFunData8 (const MemFunData8 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  object (fun_data.object),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4),
		  arg5 (fun_data.arg5),
		  arg6 (fun_data.arg6),
		  arg7 (fun_data.arg7),
		  arg8 (fun_data.arg8)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  FunDataBase *
  MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::clone () const 
  {
    return new MemFunData8 (*this);
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  void *
  MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Class            *object  = fun_data->object;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;
    Arg5              arg5    = fun_data->arg5;
    Arg6              arg6    = fun_data->arg6;
    Arg7              arg7    = fun_data->arg7;
    Arg8              arg8    = fun_data->arg8;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (object->*fun_ptr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
  
    return 0;
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  FunEncapsulation
  MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::ArgCollector::collect_args (Class *object,
												  Arg1   arg1,
												  Arg2   arg2,
												  Arg3   arg3,
												  Arg4   arg4,
												  Arg5   arg5,
												  Arg6   arg6,
												  Arg7   arg7,
												  Arg8   arg8)
  {
    return new MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,void>(fun_ptr, object,
									       arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
  }
  


  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename RetType>
  inline
  FunEncapsulation
  MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,RetType>::ArgCollector::collect_args (Class &object,
												  Arg1   arg1,
												  Arg2   arg2,
												  Arg3   arg3,
												  Arg4   arg4,
												  Arg5   arg5,
												  Arg6   arg6,
												  Arg7   arg7,
												  Arg8   arg8)
  {
    return collect_args (&object, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
  }


/* ---------------------- MemFunData9 implementation ------------------------ */
  
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::MemFunData9 (FunPtr  fun_ptr,
											Class  *object,
											Arg1    arg1,
											Arg2    arg2,
											Arg3    arg3,
											Arg4    arg4,
											Arg5    arg5,
											Arg6    arg6,
											Arg7    arg7,
											Arg8    arg8,
											Arg9    arg9) :
		  FunDataBase (&MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  object (object),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4),
		  arg5 (arg5),
		  arg6 (arg6),
		  arg7 (arg7),
		  arg8 (arg8),
		  arg9 (arg9)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::MemFunData9 (const MemFunData9 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  object (fun_data.object),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4),
		  arg5 (fun_data.arg5),
		  arg6 (fun_data.arg6),
		  arg7 (fun_data.arg7),
		  arg8 (fun_data.arg8),
		  arg9 (fun_data.arg9)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  FunDataBase *
  MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::clone () const 
  {
    return new MemFunData9 (*this);
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  void *
  MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Class            *object  = fun_data->object;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;
    Arg5              arg5    = fun_data->arg5;
    Arg6              arg6    = fun_data->arg6;
    Arg7              arg7    = fun_data->arg7;
    Arg8              arg8    = fun_data->arg8;
    Arg9              arg9    = fun_data->arg9;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (object->*fun_ptr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
  
    return 0;
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  FunEncapsulation
  MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::ArgCollector::collect_args (Class *object,
												       Arg1   arg1,
												       Arg2   arg2,
												       Arg3   arg3,
												       Arg4   arg4,
												       Arg5   arg5,
												       Arg6   arg6,
												       Arg7   arg7,
												       Arg8   arg8,
												       Arg9   arg9)
  {
    return new MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,void>(fun_ptr, object,
										    arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
  }
  


  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename RetType>
  inline
  FunEncapsulation
  MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,RetType>::ArgCollector::collect_args (Class &object,
												       Arg1   arg1,
												       Arg2   arg2,
												       Arg3   arg3,
												       Arg4   arg4,
												       Arg5   arg5,
												       Arg6   arg6,
												       Arg7   arg7,
												       Arg8   arg8,
												       Arg9   arg9)
  {
    return collect_args (&object, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
  }


/* ---------------------- MemFunData10 implementation ------------------------ */
  
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::MemFunData10 (FunPtr  fun_ptr,
												Class  *object,
												Arg1    arg1,
												Arg2    arg2,
												Arg3    arg3,
												Arg4    arg4,
												Arg5    arg5,
												Arg6    arg6,
												Arg7    arg7,
												Arg8    arg8,
												Arg9    arg9,
												Arg10   arg10) :
		  FunDataBase (&MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::thread_entry_point),
		  fun_ptr (fun_ptr),
		  object (object),
		  arg1 (arg1),
		  arg2 (arg2),
		  arg3 (arg3),
		  arg4 (arg4),
		  arg5 (arg5),
		  arg6 (arg6),
		  arg7 (arg7),
		  arg8 (arg8),
		  arg9 (arg9),
		  arg10 (arg10)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::MemFunData10 (const MemFunData10 &fun_data) :
		  FunDataBase (fun_data),
		  fun_ptr (fun_data.fun_ptr),
		  object (fun_data.object),
		  arg1 (fun_data.arg1),
		  arg2 (fun_data.arg2),
		  arg3 (fun_data.arg3),
		  arg4 (fun_data.arg4),
		  arg5 (fun_data.arg5),
		  arg6 (fun_data.arg6),
		  arg7 (fun_data.arg7),
		  arg8 (fun_data.arg8),
		  arg9 (fun_data.arg9),
		  arg10(fun_data.arg10)		  
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  FunDataBase *
  MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::clone () const 
  {
    return new MemFunData10 (*this);
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  void *
  MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::thread_entry_point (void *arg) 
  {
				     // convenience typedef, since we
				     // will need that class name
				     // several times below
    typedef MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType> ThisClass;
  
    FunEncapsulation *fun_encapsulation
      = reinterpret_cast<FunEncapsulation*>(arg);
    const ThisClass *fun_data
      = dynamic_cast<const ThisClass*> (fun_encapsulation->fun_data_base);

				     // copy the parameters
    ThisClass::FunPtr fun_ptr = fun_data->fun_ptr;
    Class            *object  = fun_data->object;
    Arg1              arg1    = fun_data->arg1;
    Arg2              arg2    = fun_data->arg2;
    Arg3              arg3    = fun_data->arg3;
    Arg4              arg4    = fun_data->arg4;
    Arg5              arg5    = fun_data->arg5;
    Arg6              arg6    = fun_data->arg6;
    Arg7              arg7    = fun_data->arg7;
    Arg8              arg8    = fun_data->arg8;
    Arg9              arg9    = fun_data->arg9;
    Arg10             arg10   = fun_data->arg10;


				     // copying of parameters is done,
				     // now we can release the lock on
				     // @p{fun_data}
    fun_data->lock.release ();

				     // call the function
    (object->*fun_ptr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
  
    return 0;
  }



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::ArgCollector::ArgCollector (FunPtr fun_ptr) :
		  fun_ptr (fun_ptr)
  {}



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  FunEncapsulation
  MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::ArgCollector::collect_args (Class *object,
													      Arg1   arg1,
													      Arg2   arg2,
													      Arg3   arg3,
													      Arg4   arg4,
													      Arg5   arg5,
													      Arg6   arg6,
													      Arg7   arg7,
													      Arg8   arg8,
													      Arg9   arg9,
													      Arg10  arg10)
  {
    return new MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,void>(fun_ptr, object,
										    arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
  }
  


  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10, typename RetType>
  inline
  FunEncapsulation
  MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,RetType>::ArgCollector::collect_args (Class &object,
													      Arg1   arg1,
													      Arg2   arg2,
													      Arg3   arg3,
													      Arg4   arg4,
													      Arg5   arg5,
													      Arg6   arg6,
													      Arg7   arg7,
													      Arg8   arg8,
													      Arg9   arg9,
													      Arg10  arg10)
  {
    return collect_args (&object, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
  }


/* ---------------------------------------------------------------- */

  inline
  FunData0<void>::ArgCollector
  encapsulate (void (*fun_ptr)())
  {
    return fun_ptr;
  }



  template <typename Arg1>
  typename FunData1<Arg1,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1))
  {
    return fun_ptr;
  }
  

  
  template <typename Arg1, typename Arg2>
  typename FunData2<Arg1,Arg2,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2))
  {
    return fun_ptr;
  }
  

  
  template <typename Arg1, typename Arg2, typename Arg3>
  typename FunData3<Arg1,Arg2,Arg3,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3))
  {
    return fun_ptr;
  }
  

  
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
  typename FunData4<Arg1,Arg2,Arg3,Arg4,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4))
  {
    return fun_ptr;
  }
  

  
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
  typename FunData5<Arg1,Arg2,Arg3,Arg4,Arg5,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5))
  {
    return fun_ptr;
  }


  
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
  typename FunData6<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6))
  {
    return fun_ptr;
  }


  
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
  typename FunData7<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7))
  {
    return fun_ptr;
  }


  
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
  typename FunData8<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8))
  {
    return fun_ptr;
  }


  
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
  typename FunData9<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5,
			       Arg6, Arg7, Arg8, Arg9))
  {
    return fun_ptr;
  }


  
  template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
  typename FunData10<Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,void>::ArgCollector
  encapsulate (void (*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5,
			       Arg6, Arg7, Arg8, Arg9, Arg10))
  {
    return fun_ptr;
  }


  
  template <class Class>
  typename MemFunData0<Class,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)())
  {
    return fun_ptr;
  }

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
  template <class Class>
  typename MemFunData0<const Class,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)() const)
  {
    return fun_ptr;
  }
#endif



  template <class Class, typename Arg1>
  typename MemFunData1<Class,Arg1,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1))
  {
    return fun_ptr;
  }

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
  template <class Class, typename Arg1>
  typename MemFunData1<const Class,Arg1,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1) const)
  {
    return fun_ptr;
  }
#endif
  

  
  template <class Class, typename Arg1, typename Arg2>
  typename MemFunData2<Class,Arg1,Arg2,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2))
  {
    return fun_ptr;
  }

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
  template <class Class, typename Arg1, typename Arg2>
  typename MemFunData2<const Class,Arg1,Arg2,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2) const)
  {
    return fun_ptr;
  }
#endif
  

  
  template <class Class, typename Arg1, typename Arg2, typename Arg3>
  typename MemFunData3<Class,Arg1,Arg2,Arg3,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3))
  {
    return fun_ptr;
  }

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
  template <class Class, typename Arg1, typename Arg2, typename Arg3>
  typename MemFunData3<const Class,Arg1,Arg2,Arg3,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3) const)
  {
    return fun_ptr;
  }
#endif
  

  
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
  typename MemFunData4<Class,Arg1,Arg2,Arg3,Arg4,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4))
  {
    return fun_ptr;
  }

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
  typename MemFunData4<const Class,Arg1,Arg2,Arg3,Arg4,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4) const)
  {
    return fun_ptr;
  }
#endif


  
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
  typename MemFunData5<Class,Arg1,Arg2,Arg3,Arg4,Arg5,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5))
  {
    return fun_ptr;
  }

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
  typename MemFunData5<const Class,Arg1,Arg2,Arg3,Arg4,Arg5,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5) const)
  {
    return fun_ptr;
  }
#endif


  
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
  typename MemFunData6<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6))
  {
    return fun_ptr;
  }

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
  typename MemFunData6<const Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const)
  {
    return fun_ptr;
  }
#endif


  
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
  typename MemFunData7<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7))
  {
    return fun_ptr;
  }

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
  typename MemFunData7<const Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) const)
  {
    return fun_ptr;
  }
#endif


  
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
  typename MemFunData8<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8))
  {
    return fun_ptr;
  }

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
  typename MemFunData8<const Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8) const)
  {
    return fun_ptr;
  }
#endif



  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
  typename MemFunData9<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9))
  {
    return fun_ptr;
  }

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
  typename MemFunData9<const Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9) const)
  {
    return fun_ptr;
  }
#endif
  

  
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
  typename MemFunData10<Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10))
  {
    return fun_ptr;
  }

#ifdef DEAL_II_TEMPL_CONST_MEM_PTR_BUG
  template <class Class, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
  typename MemFunData10<const Class,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9,Arg10,void>::ArgCollector
  encapsulate (void (Class::*fun_ptr)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10) const)
  {
    return fun_ptr;
  }
#endif
  

  
  template <typename ForwardIterator>
  typename std::vector<typename std::pair<ForwardIterator,ForwardIterator> >
  split_range (const ForwardIterator &begin,
	       const ForwardIterator &end,
	       const unsigned int     n_intervals)
  {
    typedef typename std::pair<ForwardIterator,ForwardIterator> IteratorPair;

				     // in non-multithreaded mode, we
				     // often have the case that this
				     // function is called with
				     // n_intervals==1, so have a
				     // shortcut here to handle that
				     // case efficiently
    
    if (n_intervals==1)
      return (typename std::vector<IteratorPair>
	      (1, IteratorPair(begin, end)));
    
				     // if more than one interval
				     // requested, do the full work
    const unsigned int n_elements              = distance (begin, end);
    const unsigned int n_elements_per_interval = n_elements / n_intervals;
    const unsigned int residual                = n_elements % n_intervals;
    
    std::vector<IteratorPair> return_values (n_intervals);

    return_values[0].first = begin;
    for (unsigned int i=0; i<n_intervals; ++i)
      {
	if (i != n_intervals-1) 
	  {
	    return_values[i].second = return_values[i].first;
					     // note: the cast is
					     // performed to avoid a
					     // warning of gcc that in
					     // the library `dist>=0'
					     // is checked (dist has a
					     // template type, which
					     // here is unsigned if no
					     // cast is performed)
	    std::advance (return_values[i].second,
			  static_cast<signed int>(n_elements_per_interval));
					     // distribute residual in
					     // division equally among
					     // the first few
					     // subintervals
	    if (i < residual)
	      ++return_values[i].second;
	    
	    return_values[i+1].first = return_values[i].second;
	  }
	else
	  return_values[i].second = end;
      }
    return return_values;
  }  


	    
};   // end of implementation of namespace Threads




//----------------------------   thread_management.h     ---------------------------
// end of #ifndef __deal2__thread_management_h
#endif
//----------------------------   thread_management.h     ---------------------------
