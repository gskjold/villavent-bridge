/*
 * osapi.h
 *
 *  Created on: 15. mai 2022
 *      Author: sigurd
 */

#ifndef OSAPI_H_
#define OSAPI_H_

#include <stdint.h>




/*
* Context for an element, to inherit from when implementing for an OS
*/
class OSAPI_Context
{
public:
	OSAPI_Context(){};
  virtual ~OSAPI_Context(){};
  virtual void *nativeHandle() const { return 0; }
};

/*
 *  API for creating tasks.
 *
 *
 */
class OSAPI_Task
{
public:
  OSAPI_Task(const char *name, uint32_t stacksize, uint32_t pri, int32_t core=-1 );
  virtual ~OSAPI_Task();

  // call this from sub class after created to assure virtual table is
  // complete with taskCode() method an members.
  void start();

  // Get name of task.
  const char *taskName() const;


  // Implement this in sub-class. Check m_do_end and finish by setting m_ended;
  virtual void taskCode()  = 0;

  // call to request task to end
  void requestEnd() { m_do_end=true; }

  // Check if task has ended.
  bool ended() const { return m_ended; }

  static void Sleep( uint32_t ms );

  // Get priority of task associated with this object.
  int32_t getTaskPriority();
  //int32_t getTaskAffinityMask();

   
  static int32_t GetTaskPriority(); // Get calling task priority 
  //static int32_t GetTaskAffinityMask(); // calling task affinity mask
  static int32_t GetCurrentCoreId(); // calling task core number now.

public:

protected:
  bool           m_do_end;
  bool 		     m_ended;
  OSAPI_Context *m_context;
};

/*
 *  C++ API for Semaphores.
 *
 */
class OSAPI_Semaphore
{
public:
  OSAPI_Semaphore( const char *name, int32_t initcount=1 );
  virtual ~OSAPI_Semaphore();

  // lock semaphore. timeout in milliseconds. -1 for infinite
  // returns negative on error/timeout, and 0 on success.
  int32_t take(int32_t timeout_ms );
  int32_t give();

protected:
  OSAPI_Context *m_context;
};



#endif /* MAIN_OSAPI_H_ */
