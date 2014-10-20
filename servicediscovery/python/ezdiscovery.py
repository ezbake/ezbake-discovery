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

"""Register and locate services within ezDiscovery."""

import optparse
import sys

from kazoo import client

NAMESPACE = 'ezDiscovery'
COMMON_APP_NAME = 'common_services'
ENDPOINTS = 'endpoints'
SECURITY = 'security'
SECURITY_ID = 'security_id'
zk = None


class NotConnected(Exception):
    """Thrown when an api call is sent before the Zookeeper client is setup."""
    pass


def connect(host):
    """Create a connection to Zookeeper for use in discovery calls."""
    global zk
    zk = client.KazooClient(hosts=host)
    zk.start()


def disconnect():
    """Disconnect from Zookeeper if the module has a connection."""
    if zk:
        zk.stop()


def _verify_connected():
    """Shortcut method to validate Zookeeper connection."""
    if not zk:
        raise NotConnected('Active connection required for operation.')


def register_endpoint(app_name, service_name, host, port):
    """Register an endpoint with Zookeeper."""
    _verify_connected()
    zk.ensure_path('/'.join([
        NAMESPACE,
        app_name,
        service_name,
        ENDPOINTS,
        host + ':' + str(port)
    ]))


def register_common_endpoint(service_name, host, port):
    """Register a common endpoint under the default application name."""
    register_endpoint(COMMON_APP_NAME, service_name, host, port)


def _recurse_deletion(path_parts):
    """Moves up the tree of the given path parts deleting if empty.

    NOTE: Will not delete a root node.
    """
    if len(path_parts) > 1:
        path = '/'.join(path_parts)
        if zk.exists(path) and not len(zk.get_children(path)):
            zk.delete(path)
            _recurse_deletion(path_parts[:-1])


def unregister_endpoint(app_name, service_name, host, port):
    """Unregister and endpoint with Zookeeper."""
    _verify_connected()
    _recurse_deletion([
        NAMESPACE,
        app_name,
        service_name,
        ENDPOINTS,
        host + ':' + str(port)
    ])


def unregister_common_endpoint(service_name, host, port):
    """Unregister a common endpoint under the default application name."""
    unregister_endpoint(COMMON_APP_NAME, service_name, host, port)


def _get_children(path):
    """Shortcut method to return the children on the given path."""
    _verify_connected()
    if zk.exists(path):
        return zk.get_children(path)
    else:
        return []


def get_applications():
    """Get a list of applications registered in Zookeeper."""
    return _get_children(NAMESPACE)


def get_services(app_name):
    """Get a list services by the given application name."""
    return _get_children('/'.join([NAMESPACE, app_name]))


def get_common_services():
    """Get a list services under the common application name."""
    return get_services(COMMON_APP_NAME)


def get_endpoints(app_name, service_name):
    """Get a list of endpoints by the given application and service name."""
    return _get_children(
        '/'.join([NAMESPACE, app_name, service_name, ENDPOINTS])
    )


def get_common_endpoints(service_name):
    """Get a list of endpoints from the common application name and given
    service name.
    """
    return get_endpoints(COMMON_APP_NAME, service_name)


def is_service_common(service_name):
    """Checks if the given service name is in the common services application.

    NOTE: Returns false if the service does not exist.
    """
    _verify_connected()
    return bool(zk.exists('/'.join([
        NAMESPACE,
        COMMON_APP_NAME,
        service_name
    ])))


def set_security_id_for_application(app_name, security_id):
    """Set the security id for the given application."""
    # TODO(jmears): Should these be cleaned up when an application is
    # unregistered?
    _verify_connected()
    path = '/'.join([
        NAMESPACE,
        app_name,
        SECURITY,
        SECURITY_ID
    ])
    zk.ensure_path(path)
    zk.set(path, security_id)


def set_security_id_for_common_service(service_name, security_id):
    """Set the security id for the given common service."""
    # TODO(jmears): Should these be cleaned up when the service is
    # unregistered?
    _verify_connected()
    set_security_id_for_application(
        '/'.join([COMMON_APP_NAME, service_name]),
        security_id
    )


def get_security_id_for_application(app_name):
    """Fetch the security id for the given application."""
    _verify_connected()
    try:
        return zk.get('/'.join([
            NAMESPACE,
            app_name,
            SECURITY,
            SECURITY_ID
        ]))[0]
    except client.NoNodeError:
        pass


def get_security_id_for_common_service(service_name):
    """Fetch the security id for the given common service."""
    _verify_connected()
    return get_security_id_for_application(
        '/'.join([COMMON_APP_NAME, service_name])
    )


def _arg_count(args, number, message='Invalid arguments.'):
    """Counts the arguments given and exits with failed status if needed.

    Really just a convenience method for the main method, not part of the
    discovery API.
    """
    if len(args) < number:
        print message
        sys.exit(1)


def _invalid_action(action=''):
    """Prints an error message and exits."""
    if action:
        print 'Invalid action: ' % (action)
    else:
        print 'Action not specified.'
    sys.exit(1)

ACTIONS = {
    'register': {
        'args': [5, 'Must provide app name, service name, host and port.'],
        'method': register_endpoint
    },
    'unregister': {
        'args': [5, 'Must provide app name, service name, host and port.'],
        'method': unregister_endpoint
    },
    'list-applications': {
        'method': get_applications
    },
    'list-services': {
        'args': [2, 'Must provide an app name.'],
        'method': get_services
    },
    'list-common-services': {
        'method': get_common_services
    },
    'list-endpoints': {
        'args': [3, 'Must provide app name, service name, host and port.'],
        'method': get_endpoints
    },
    'list-common-endpoints': {
        'args': [2, 'Must provide a common service name.'],
        'method': get_common_endpoints
    },
    'is-service-common': {
        'args': [2, 'Must provide a service name.'],
        'method': is_service_common
    },
    'application-set-security-id': {
        'args': [3, 'Must provide an app name and security id.'],
        'method': set_security_id_for_application
    },
    'application-get-security-id': {
        'args': [2, 'Must provide an app name.'],
        'method': get_security_id_for_application
    },
    'common-service-set-security-id': {
        'args': [3, 'Must provide a service name and security id.'],
        'method': set_security_id_for_common_service
    },
    'common-service-get-security-id': {
        'args': [2, 'Must provide a service name.'],
        'method': get_security_id_for_common_service
    },
}


def main():
    """Module will act as a command line utility if not imported as a module in
    another application.
    """
    parser = optparse.OptionParser(
        usage='usage: %prog [options] ACTION arg1 arg2 ...'
    )
    parser.add_option(
        '-z',
        '--zookeeper',
        default='localhost:2181',
        help='Zookeeper location (host:port).'
    )
    options, args = parser.parse_args()

    if not args:
        _invalid_action()
    action = args[0]

    if action in ACTIONS:
        action = ACTIONS[action]
        if 'args' in action:
            _arg_count(args, action['args'][0], action['args'][1])
        connect(options.zookeeper)
        method_args = args[1:]
        result = None
        if method_args:
            result = action['method'](*method_args)
        else:
            result = action['method']()
        if result is not None:  # Some commands return a boolean.
            if isinstance(result, list):
                for i in result:
                    print i
            else:
                print result
    else:
        print 'Invalid action: ' % (action)
        sys.exit(1)

    disconnect()

if __name__ == '__main__':
    main()
