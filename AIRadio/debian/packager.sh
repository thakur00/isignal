#!/bin/bash
#
# Copyright 2013-2022 iSignal Research Labs Pvt Ltd.
#
# This file is part of isrRAN
#
# isrRAN is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# isrRAN is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# A copy of the GNU Affero General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#

RELEASE=19.06
DISTRO=disco
COMMIT=eda7ca69a09933526e9318bcf553af0dc0b81598
REL_FLAG=releases

echo 'Packaging isrRAN release' $RELEASE 'for Ubuntu' $DISTRO 

# Make build dir for the package
BUILD_DIR=~/build-area/isrran_$RELEASE/$REL_FLAG/$DISTRO
mkdir -p $BUILD_DIR

# Make tarball of the package source
pushd ~/isrRAN
git archive $COMMIT -o $BUILD_DIR/isrran_$DISTRO.tar.gz
popd

# Copy original tarball
cp ~/build-area/isrran_$RELEASE/$REL_FLAG/isrran_$RELEASE.orig.tar.gz $BUILD_DIR

mkdir $BUILD_DIR/isrRAN
pushd $BUILD_DIR/isrRAN
tar -vxzf ../isrran_$DISTRO.tar.gz
popd
