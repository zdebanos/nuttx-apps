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

.NOTPARALLEL: 

SHVDIR = shv-libs4c

$(SHVDIR):
	git clone https://github.com/silicon-heaven/shv-libs4c $(SHVDIR)
	
# Compile the library. Also pass all the NuttX Makefile variables.
# I've stumbled upon serious problems with creating the ULUT test binaries
# so this quick sed hack won't compile the test binaries.
compile-libshv: $(SHVDIR)
	( cd $(SHVDIR) && git switch dev-zdebanos-dirty-13-07 && \
	  git submodule update --recursive --init && \
	  sed -i '1s/^/#/' ./submodule/ulut/Makefile.omk )
	make -C $(SHVDIR) CONFIG_SHV_LIBS4C_PLATFORM="nuttx" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		CXXFLAGS="$(CXXFLAGS)" \
		LD="$(LD)" \
	    LDFLAGS="$(LDFLAGS)"

context:: compile-libshv

depend:: compile-libshv
	
clean::
	$(call DELDIR, $(SHVDIR))

distclean::
	make -C $(SHVDIR) distclean
	$(call DELDIR, $(SHVDIR))

include $(APPDIR)/Application.mk
