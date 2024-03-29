#!/usr/bin/env python
#   Copyright (C) 2013-2014 Computer Sciences Corporation
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

"""Setup script for installing ezDiscovery as a module."""

from distutils.core import setup

setup(
    name='ezdiscovery',
    version='0.2',
    description='Python module for talking to ezDiscovery service.',
    author='Jordon Mears',
    author_email='jmears@t-sciences.com',
    install_requires=['kazoo'],
    py_modules=['ezdiscovery']
)
