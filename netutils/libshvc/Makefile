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

# Set this to 1 for development purposes
COMPILE_EVERYTIME = 1
SHVDIR = shv-libs4c
TARGETSHVTREE = $(SHVDIR)/_compiled/libshvtree.a
TARGETSHVCHAINPACK = $(SHVDIR)/_compiled/libshvchainpack.a
TARGETULUT = $(SHVDIR)/_compiled/libulut.a
TARGETSSHV = $(TARGETSHVTREE) $(TARGETSHVCHAINPACK) $(TARGETULUT)

$(SHVDIR):
	git clone https://github.com/silicon-heaven/shv-libs4c $(SHVDIR)
	
# Compile the library. Also pass all the NuttX Makefile variables.
# But only in case of a newer library, non-existing _compiled directory
# or MY_DEBUG set
download-and-generate-headers: $(SHVDIR)
	- $(Q) ( cd $(SHVDIR) && git fetch origin dev-zdebanos ) > /dev/null 2>&1
	$(Q) if [ $(COMPILE_EVERYTIME) -eq 1 ] || \
	        [ "$$(cd $(SHVDIR) && git rev-parse HEAD)" != "$$(cd $(SHVDIR) && git rev-parse origin/dev-zdebanos)" ] || \
			[ ! -d "./$(SHVDIR)/_compiled" ]; then \
		( cd $(SHVDIR); git pull; git switch dev-zdebanos; git submodule update --recursive --init ) > /dev/null 2>&1; \
		make -C $(SHVDIR) CONFIG_SHV_LIBS4C_PLATFORM="nuttx" \
			CC="$(CC)" \
			CXX="$(CXX)" \
			CFLAGS="$(CFLAGS)" \
			CXXFLAGS="$(CXXFLAGS)" \
			LD="$(LD)" \
			LDFLAGS="$(LDFLAGS)" \
			CONFIG_OC_ULUT_TESTS=n include-pass; \
	fi

$(TARGETSSHV): $(SHVDIR)
	@echo "nazdarrrrrrrrrrrr"
	@echo $(CFLAGS)
	make -C $(SHVDIR) CONFIG_SHV_LIBS4C_PLATFORM="nuttx" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		CXXFLAGS="$(CXXFLAGS)" \
		LD="$(LD)" \
		LDFLAGS="$(LDFLAGS)" \
		CONFIG_OC_ULUT_TESTS=n

context:: download-and-generate-headers

depend:: $(TARGETSSHV)
	
distclean::
	make -C $(SHVDIR) distclean
	$(call DELDIR, $(SHVDIR))

include $(APPDIR)/Application.mk
