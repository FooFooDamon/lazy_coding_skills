/*
 * Just the default definition of __VER__.
 *
 * Copyright (c) 2023 Man Hung-Coeng <udc577@126.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef ____VER___H__
#define ____VER___H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NOTES:
 *
 * It's recommended to assign the version number generated by VCS to __VER__.
 * Consider using __VER__ together with __ver__.mk and c_and_cpp.mk
 * in ../../makefile directory.
 *
 * Besides, you might want to define more version macros, for example:
 *  /project/root/directory # __VER__ automatically generated by VCS.
 *      |-- apps # {BUNDLE/SUITE}_VERSION or whatever name you like.
 *      |   |-- module1 # {MODULE/COMPONENT}_VERSION or whatever name you like.
 *      |   |   |-- file1.c # {MONOMER/UNIT}_VERSION or whatever name you like.
 *      .   .   .
 *      .   .   .
 *      .   .   .
 *      |   |   `-- fileN.c # Similar to file1.c.
 *      .   .
 *      .   .
 *      .   .
 *      |   `-- moduleN # Similar to module1.
 *      |-- drivers # Similar to apps.
 *      `-- kernel  # Similar to apps.
 *  Note that version macros above (except for __VER__) are optional,
 *  and might need to be modified manually or through a script on each release.
 *  Just do it the way you like.
 */

#ifndef __VER__
#define __VER__         "<none>"
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef ____VER___H__ */

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> 2023-04-16, Man Hung-Coeng <udc577@126.com>:
 *  01. Create.
 *
 * >>> 2023-06-23, Man Hung-Coeng <udc577@126.com>:
 *  01. Change the default value of __VER__ from 0123456789abcdef to <none>.
 */

