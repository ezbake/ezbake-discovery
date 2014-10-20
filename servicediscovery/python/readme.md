ezDiscovery for Python
======================
A simple Python module for talking the EzBake Discovery service. This module
can be used as an imported library in a larger application or as a simple
command line tool.

Requirements
------------
This module depends on `kazoo`: https://pypi.python.org/pypi/kazoo

Installing as a Module
----------------------
ezDiscovery installs like any other Python module.

    python setup.py install

NOTE: You may need to use `sudo` if you are installing to system Python.

Running on the Command Line
---------------------------
As long as you have the dependencies installed you can play with ezDiscovery on
the command line. The command supports a list of actions:

  - `python ezdiscovery.py register APPNAME SERVICENAME HOST PORT`
  - `python ezdiscovery.py unregister APPNAME SERVICENAME HOST PORT`
  - `python ezdiscovery.py list-applications`
  - `python ezdiscovery.py list-services APPNAME`
  - `python ezdiscovery.py list-endpoints APPNAME SERVICENAME`

Execute `python ezdiscovery.py --help` for more details as well.

Running Tests
-------------
The `kazoo` library will mock out a Zookeeper instance for running tests
against but is does need to know where Zookeeper is installed in order to pull
in libraries. You may need export an environment variable called
`ZOOKEEPER_PATH` in order for the tests to run.

    python test.py

Developing
----------
To work with this codebase you need a Linux machine of some kind (deb or rpm
based). Once you have a machine up with Python installed, you need to run
`./scripts/bootstrap.sh` to install all dependencies.

    ./scripts/bootstrap.sh

This will setup a python virtual environment containing all Python dependencies
in `~/.ez_dev/py`. In order to work with the module you will need to activate
this virtual environment on the command line:

    source ~/.ez_dev/py/bin/activate

The `./scripts/bootstrap.sh` script also installs Zookeeper into
`~/.ez_dev/zookeeper`. You can start, stop and manage the Zookeeper instance
with:

    ./scripts/zoo.sh

Also to validate your Python code style before submitting code you can run
`./scripts/check-style.sh`. This will run pep8 validation against the codebase.

    ./scripts/check-style.sh
