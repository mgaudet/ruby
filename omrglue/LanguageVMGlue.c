/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2014, 2016
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 *******************************************************************************/

#include "ruby/ruby.h"

#include "omr.h"
#include "omrvm.h"
#include "rbomrinit.h"

omr_error_t
OMR_Glue_BindCurrentThread(OMR_VM *omrVM, const char *threadName, OMR_VMThread **omrVMThread)
{
	omr_error_t rc = OMR_ERROR_NONE;

	OMR_VMThread *currentThread = omr_vmthread_getCurrent(omrVM);
	if (NULL == currentThread) { /* not already attached */
		omrthread_t self = NULL;
		if (0 == omrthread_attach(&self)) {
			rb_thread_t *rubyThread = NULL;

			rc = OMR_Glue_AllocLanguageThread(omrVM->_language_vm, (void **)&rubyThread);
			if (OMR_ERROR_NONE != rc) {
				goto done;
			}

			/* This implicitly links the OMR thread to the ruby thread, so don't need to
			 * explicitly call OMR_Glue_LinkLanguageThreadToOMRThread().
			 */
			rc = OMR_Thread_FirstInit(omrVM, self, (void *)rubyThread, &rubyThread->_omrVMThread, threadName);
			if (OMR_ERROR_NONE != rc) {
				OMR_Glue_FreeLanguageThread((void *)rubyThread);
				goto done;
			}
			/* success */
			*omrVMThread = rubyThread->_omrVMThread;
done:
			if (OMR_ERROR_NONE != rc) {
				/* error cleanup */
				omrthread_detach(self);
			} else {
				/* Initialize monitors */
				rc = omrthread_monitor_init_with_name(&rubyThread->publicFlagsMutex, 0, "Thread flags mutex");
			}
		} else {
			rc = OMR_ERROR_FAILED_TO_ATTACH_NATIVE_THREAD;
		}
	} else { /* already attached */
		omr_vmthread_reattach(currentThread, threadName);
		*omrVMThread = currentThread;
	}
	return rc;
}

omr_error_t
OMR_Glue_UnbindCurrentThread(OMR_VMThread *omrVMThread)
{
	omr_error_t rc = OMR_ERROR_NONE;

	if (NULL == omrVMThread) {
		rc = OMR_THREAD_NOT_ATTACHED;
	} else {
		omrthread_t self = omrVMThread->_os_thread;

		if (1 == omrVMThread->_attachCount) {
			/* cache vmThread members so that we can use them after the vmThread is destroyed */
			void *languageThread = omrVMThread->_language_vmthread;

			rc = OMR_Thread_LastFree(omrVMThread);
			if (OMR_ERROR_NONE != rc) {
				goto done;
			}
			rc = OMR_Glue_FreeLanguageThread(languageThread);
			if (OMR_ERROR_NONE != rc) {
				goto done;
			}
			omrthread_detach(self);
		} else {
			omr_vmthread_redetach(omrVMThread);
		}
	}
done:
	return rc;
}

omr_error_t
OMR_Glue_AllocLanguageThread(void *languageVM, void **languageThread)
{
	omr_error_t rc = OMR_ERROR_NONE;
	rb_thread_t *rubyThread = (rb_thread_t *)ruby_mimmalloc(sizeof(rb_thread_t));
	if (NULL == rubyThread) {
		rc = OMR_ERROR_OUT_OF_NATIVE_MEMORY;
	} else {
		MEMZERO(rubyThread, rb_thread_t, 1);
		rubyThread->bindAllocatedFlag = 1;
		rubyThread->vm = (rb_vm_t *)languageVM;
		*languageThread = (void *)rubyThread;
	}
	return rc;
}

omr_error_t
OMR_Glue_FreeLanguageThread(void *languageThread)
{
	if (NULL != languageThread) {
		rb_thread_t *rubyThread = (rb_thread_t *)languageThread;
		if (rubyThread->bindAllocatedFlag) {
			omrthread_monitor_destroy(rubyThread->publicFlagsMutex);
			ruby_mimfree(rubyThread);
		}
	}
	return OMR_ERROR_NONE;
}

omr_error_t
OMR_Glue_LinkLanguageThreadToOMRThread(void *languageThread, OMR_VMThread *omrVMThread)
{
	if (NULL != languageThread) {
		rb_thread_t *rubyThread = (rb_thread_t *)languageThread;
		rubyThread->_omrVMThread = omrVMThread;
	}
	return OMR_ERROR_NONE;
}

