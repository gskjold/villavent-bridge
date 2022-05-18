#include "osapi.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <string>

using namespace std;

/* ******************************************************************* */
class TaskContext : public OSAPI_Context
{
  xTaskHandle  m_handle;
  string       m_name;
  uint32_t     m_stacksize;
  int32_t 	   m_pri;
  int32_t      m_core;
public:

  TaskContext(const char *name,uint32_t stacksize, int32_t pri, int32_t core ):m_handle(0),m_name(name), m_stacksize(stacksize), m_pri(pri), m_core(core){};
  virtual ~TaskContext() {};
  xTaskHandle getHandle() { return m_handle; }
  const char *name() const { return m_name.c_str(); }
  uint32_t stacksize()const { return m_stacksize;}
  int32_t pri() const { return m_pri;}
  int32_t core() const { return m_core; }

  virtual void *nativeHandle() const { return m_handle; }

  static TaskContext* Cast( OSAPI_Context *from ) { return static_cast<TaskContext*>( from ); }

  friend class OSAPI_Task;
};

/* ******************************************************************* */
// Static that is given to the Task create call, and that will call
// the member function taskCode() for execution of task.
static void TaskCode( OSAPI_Task * param )
{
  // Call the virtual method for the task that will typically loop and
  // execute some stuff.
  param->taskCode();

  //printf("\nTask %s exit", param->taskName() );
  //param->m_ended=true;
}

/* ******************************************************************* */
/*void OSAPI_Task::taskCode()
{
  while ( !m_do_end )
  {
	printf("\nThis method needs implementation and should never be called.");
	Sleep(2000);
  }
  m_ended = true;
};*/

/* ******************************************************************* */
OSAPI_Task::OSAPI_Task( const char *name, uint32_t stacksize, uint32_t pri, int32_t core )
 : m_do_end(false), m_ended(false),
   m_context(0)
{
  TaskContext *ct = new TaskContext(name,stacksize,pri,core);
  m_context = ct;
}

/* ******************************************************************* */
void OSAPI_Task::start()
{
  TaskContext *ct = TaskContext::Cast( m_context );

  BaseType_t xcoreid = tskNO_AFFINITY; // this means any core.
  int32_t core = ct->core();
  if ( core>=0 && core<=1 )
    xcoreid = core;

  //BaseType_t res =
  xTaskCreatePinnedToCore( (TaskFunction_t)TaskCode, ct->name(), ct->stacksize(), this, ct->pri(), &TaskContext::Cast( m_context )->m_handle, xcoreid );

  //if ( ct->affinity()!= -1 )
  //{
  //  vTaskCoreAffinitySet(  TaskContext::Cast( m_context )->m_handle, ct->affinity() );
  //}

  //printf("\n%s started with res %d", ct->name(), res );

}


/* ******************************************************************* */
OSAPI_Task::~OSAPI_Task()
{
  // This must be refined with task stopp first
  vTaskDelete( TaskContext::Cast( m_context )->getHandle() );

  delete m_context;
}

/* ******************************************************************* */
const char *OSAPI_Task::taskName() const
{
  return pcTaskGetName( TaskContext::Cast( m_context )->getHandle() );
}



/* ******************************************************************* */
void OSAPI_Task::Sleep( uint32_t ms )
{
  vTaskDelay( ms / portTICK_PERIOD_MS );
}


/* ******************************************************************* */
int32_t OSAPI_Task::getTaskPriority()
{
  return uxTaskPriorityGet( TaskContext::Cast( m_context )->getHandle()  );
}

/* ******************************************************************* */
// Get calling task priority
int32_t OSAPI_Task::GetTaskPriority()
{
  return uxTaskPriorityGet( NULL  );
}

/* ******************************************************************* */
//int32_t OSAPI_Task::getTaskAffinityMask()
//{
//   vTaskCoreAffinityGet( TaskContext::Cast( m_context )->getHandle() );
//}

//* ******************************************************************* */
//int32_t OSAPI_Task::GetTaskAffinityMask()
//{
//  vTaskCoreAffinityGet( TaskContext::Cast( m_context )->getHandle() );
//}

/* ******************************************************************* */
int32_t OSAPI_Task::GetCurrentCoreId() // calling task core number now.
{
  return xPortGetCoreID();
}

/* ******************************************************************* */
class SemContext : public OSAPI_Context
{
  xSemaphoreHandle   m_handle;
  std::string		 m_name;
public:

  SemContext(const char *name):m_handle(0), m_name(name) {};
  virtual ~SemContext() {};
  xSemaphoreHandle getHandle() { return m_handle; }

  virtual void *nativeHandle() const { return m_handle; }

  static SemContext* Cast( OSAPI_Context *from ) { return static_cast<SemContext*>( from ); }

  friend class OSAPI_Semaphore;
};

/* ******************************************************************* */
OSAPI_Semaphore::OSAPI_Semaphore( const char *name, int32_t initcount )
: m_context(0)
{
  SemContext *ct = new SemContext(name);
  m_context = ct;
  ct->m_handle = xSemaphoreCreateCounting( 1, initcount);

}
/* ******************************************************************* */
OSAPI_Semaphore::~OSAPI_Semaphore()
{
  // This must be refined with task stopp first
  vTaskDelete( SemContext::Cast( m_context )->getHandle() );

  delete m_context;
}

/* ******************************************************************* */
int32_t OSAPI_Semaphore::take(int32_t timeout_ms )
{
  TickType_t ticks = timeout_ms<0 ? portMAX_DELAY : timeout_ms / portTICK_PERIOD_MS;

  if ( xSemaphoreTake( SemContext::Cast( m_context )->getHandle(), ticks )==pdTRUE )
    return 0;
  else
    return -1;
}
 /* ******************************************************************* */
int32_t OSAPI_Semaphore::give()
{
  if ( xSemaphoreGive( SemContext::Cast( m_context )->getHandle() )==pdTRUE )
    return 0;
  else
    return -1;
}
