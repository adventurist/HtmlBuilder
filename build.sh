# Copyright (c) 2013 Sébastien Rombauts (sebastien.rombauts@gmail.com)
#
# Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
# or copy at http://opensource.org/licenses/MIT)
mkdir build
cd build
cmake ..
cmake --build .
ctest .
