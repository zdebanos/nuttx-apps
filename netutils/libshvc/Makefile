#############################################################################
# apps/netutils/libshvc/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
#############################################################################

include $(APPDIR)/Make.defs

SHVDIR := shv-libs4c

# Commit hash of the supported shv-libs4c revision
SHV_LIBS4C_COMMIT_HASH := 555a008e57f2735b7d6fc598154c812821d17e74

# Define build parameters for the shv-libs4c makefile.
# Somehow, the build system in this phase defines __KERNEL__, but ulut
# does not like this at all.
MAKE_ARGS = CC="$(CC)" CXX="$(CXX)" CFLAGS="$(filter-out -D__KERNEL__,$(CFLAGS))" \
            CXXFLAGS="$(CXXFLAGS)" LD="$(LD)"
			LDFLAGS="$(LDFLAGS)" CONFIG_OC_ULUT_TESTS=n

# Where to look for the extra object files
EXTOBJS = $(wildcard ./$(SHVDIR)/_build/user/libs4c/libshvtree/*.o)
EXTOBJS += $(wildcard ./$(SHVDIR)/_build/user/libs4c/libshvchainpack/*.o)
EXTOBJS += $(wildcard ./$(SHVDIR)/_build/user/submodule/ulut/ulut/*.o)

$(SHVDIR):
	$(call CLONE, https://github.com/silicon-heaven/shv-libs4c, $(SHVDIR),)
	( cd $(SHVDIR) && \
	  git submodule update --recursive --init && \
	  git checkout $(SHV_LIBS4C_COMMIT_HASH)) > $(EMPTYFILE) 2>&1
	
compile-libshv: $(SHVDIR)
	make -C $< CONFIG_SHV_LIBS4C_PLATFORM="nuttx" library-pass $(MAKE_ARGS)

context:: $(SHVDIR)
	make -C $< CONFIG_SHV_LIBS4C_PLATFORM="nuttx" default-config include-pass $(MAKE_ARGS)

depend:: compile-libshv
	
distclean::
	make -C $(SHVDIR) distclean
	$(call DELDIR, $(SHVDIR))

include $(APPDIR)/Application.mk
