/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2017, 2017
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


#ifndef RUBY_COMPILATION_ASSUMPTIONS
#define RUBY_COMPILATION_ASSUMPTIONS
/**
 * Compilation assumptions allow you to access VM data with no synchronization,
 * if and only if the VM can promise to notify the JIT compiler when the data 
 * becomes invalid. 
 *
 * There are two types of compilation assumptions in this design: Accessor
 * assumptions and CompilationSuccess Assumptions. 
 *
 * - Accessor Assumptions proxy the contained class; When accessed, they first
 *   check their invalidation flag, and if it is set, they throw an exception. 
 * - CompilationSuccess assumptions must remain true throughout the compilation, 
 *   or else they compilation will be aborted. These can be used to check guarded
 *   specializations that will always fail once the compilation is executed. 
 *
 * Compilation Assumptions are registered in a Compilation Assumption Table (CAT). This is 
 * where invalidation occurs. 
 *
 * Note: Accessor Assumptions should check for others guarding same value when being 
 * created and get the valid bit from there. Accessor assumptions must survive the whole
 * lifetime of the compilation in order to ensure an invalidation once delivered persists
 * the length of the compilation. 
 */
 

#endif
