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

"""Simple tests for ezDiscovery."""

import copy
import getpass
import os
import unittest

from kazoo import testing

import ezdiscovery


class EzDiscoveryTest(testing.KazooTestCase):
    """Basic set of tests for ezDiscovery."""

    def setUp(self):
        """Replace the Zookeeper client on the module."""
        super(EzDiscoveryTest, self).setUp()
        ezdiscovery.zk = self.client

    def tearDown(self):
        """Clean up the Zookeeper entries."""
        self.client.delete(ezdiscovery.NAMESPACE, recursive=True)

    def test_register_endpoint(self):
        """Register an endpoint and make sure it ends up in Zookeeper."""
        ezdiscovery.register_endpoint('foo', 'bar', 'localhost', 8080)
        path = '/'.join([
            ezdiscovery.NAMESPACE,
            'foo',
            'bar',
            ezdiscovery.ENDPOINTS,
            'localhost:' + str(8080)
        ])
        self.assertTrue(self.client.exists(path))

    def test_register_common_endpoint(self):
        """Register a common endpoint and make sure it ends up in Zookeeper."""
        ezdiscovery.register_common_endpoint('bar', 'localhost', 8080)
        path = '/'.join([
            ezdiscovery.NAMESPACE,
            ezdiscovery.COMMON_APP_NAME,
            'bar',
            ezdiscovery.ENDPOINTS,
            'localhost:' + str(8080)
        ])
        self.assertTrue(self.client.exists(path))

    def test_unregister_endpoint(self):
        """Register and unregister an endpoint and make sure it is gone."""
        path_parts = [
            ezdiscovery.NAMESPACE,
            'foo',
            'bar',
            ezdiscovery.ENDPOINTS,
            'localhost:' + str(8080)
        ]
        path = '/'.join(path_parts)
        self.client.ensure_path(path)

        ezdiscovery.unregister_endpoint('foo', 'bar', 'localhost', 8080)
        self.assertFalse(self.client.exists(path))

        # Make sure the parent paths got cleared out as well (except the top
        # namespace).
        self.assertFalse(self.client.exists('/'.join(path_parts[:-1])))
        self.assertFalse(self.client.exists('/'.join(path_parts[:-2])))
        self.assertFalse(self.client.exists('/'.join(path_parts[:-3])))
        self.assertTrue(self.client.exists('/'.join(path_parts[:-4])))

    def test_unregister_common_endpoint(self):
        """Register and unregister a common endpoint and make sure it is gone.
        """
        # Simple test here since this calls unregister_endpoint.
        path_parts = [
            ezdiscovery.NAMESPACE,
            ezdiscovery.COMMON_APP_NAME,
            'bar',
            ezdiscovery.ENDPOINTS,
            'localhost:' + str(8080)
        ]
        path = '/'.join(path_parts)
        self.client.ensure_path(path)
        ezdiscovery.unregister_common_endpoint('bar', 'localhost', 8080)
        self.assertFalse(self.client.exists(path))

    def test_unregister_recursion(self):
        """Test that when multiple endpoints get made and some removed the tree
        of endpoints stays correct.
        """
        base_path_parts = [
            ezdiscovery.NAMESPACE,
            'foo',
            'bar',
            ezdiscovery.ENDPOINTS
        ]
        base_path = '/'.join(base_path_parts)

        first_endpoint_path_parts = copy.deepcopy(base_path_parts)
        first_endpoint_path_parts.append('localhost:8000')
        first_endpoint_path = '/'.join(first_endpoint_path_parts)
        self.client.ensure_path(first_endpoint_path)

        second_endpoint_path_parts = copy.deepcopy(base_path_parts)
        second_endpoint_path_parts.append('localhost:8888')
        second_endpoint_path = '/'.join(second_endpoint_path_parts)
        self.client.ensure_path(second_endpoint_path)

        # Unregister the first endpoint.
        ezdiscovery.unregister_endpoint('foo', 'bar', 'localhost', 8000)
        self.assertFalse(self.client.exists(first_endpoint_path))
        self.assertTrue(self.client.exists(second_endpoint_path))
        self.assertTrue(self.client.exists(base_path))

        ezdiscovery.unregister_endpoint('foo', 'bar', 'localhost', 8888)

        # Make sure the parent paths got cleared out as well (except the top
        # namespace).
        self.assertFalse(self.client.exists('/'.join(base_path_parts)))
        self.assertFalse(self.client.exists('/'.join(base_path_parts[:-1])))
        self.assertFalse(self.client.exists('/'.join(base_path_parts[:-2])))
        self.assertTrue(self.client.exists('/'.join(base_path_parts[:-3])))

    def test_get_applications(self):
        """Test application list."""
        # Create a few application endpoints.
        ezdiscovery.register_endpoint('foo', 'bar', 'localhost', 8000)
        ezdiscovery.register_endpoint('harry', 'sally', 'localhost', 8080)
        self.assertEqual(2, len(ezdiscovery.get_applications()))

    def test_get_services(self):
        """Test the application services list."""
        # Create a few application endpoints.
        ezdiscovery.register_endpoint('foo', 'bar', 'localhost', 8000)
        ezdiscovery.register_endpoint('foo', 'baz', 'localhost', 8001)
        ezdiscovery.register_endpoint('harry', 'sally', 'localhost', 8080)

        # Make sure it returns the right count for a single service.
        self.assertEqual(2, len(ezdiscovery.get_services('foo')))

        self.assertEqual(1, len(ezdiscovery.get_services('harry')))
        self.assertEqual('sally', ezdiscovery.get_services('harry')[0])

    def test_get_common_services(self):
        """Test fetching common services."""
        # Make a few common services and and an external, ensure they return
        # properly.
        ezdiscovery.register_common_endpoint('foo', 'localhost', 8000)
        ezdiscovery.register_common_endpoint('bar', 'localhost', 8001)
        ezdiscovery.register_endpoint('harry', 'sally', 'localhost', 8080)
        self.assertEqual(2, len(ezdiscovery.get_common_services()))

    def test_get_endpoints(self):
        """Test endpoint list fetching."""
        # Create a few application endpoints.
        ezdiscovery.register_endpoint('foo', 'bar', 'localhost', 8000)
        ezdiscovery.register_endpoint('foo', 'bar', 'localhost', 8001)
        ezdiscovery.register_endpoint('harry', 'sally', 'localhost', 8080)
        self.assertEqual(2, len(ezdiscovery.get_endpoints('foo', 'bar')))

    def test_get_common_endpoints(self):
        """Test fetching common endpoints."""
        # Create a few common endpoints and one not, test results.
        ezdiscovery.register_common_endpoint('foo', 'localhost', 8000)
        ezdiscovery.register_common_endpoint('foo', 'localhost', 8001)
        ezdiscovery.register_endpoint('harry', 'sally', 'localhost', 8080)
        self.assertEqual(2, len(ezdiscovery.get_common_endpoints('foo')))
        self.assertEquals(0, len(ezdiscovery.get_common_endpoints('sally')))

    def test_is_service_common(self):
        """Ensure only common services return true."""
        # Test one that does not exist.
        self.assertFalse(ezdiscovery.is_service_common('foo'))
        ezdiscovery.register_common_endpoint('foo', 'localhost', 8000)
        self.assertTrue(ezdiscovery.is_service_common('foo'))
        ezdiscovery.register_endpoint('harry', 'sally', 'localhost', 8080)
        self.assertFalse(ezdiscovery.is_service_common('sally'))

    def test_set_security_id_for_application(self):
        """Ensure security id's get set for applications."""
        ezdiscovery.register_endpoint('foo', 'bar', 'localhost', 8000)
        ezdiscovery.set_security_id_for_application('foo', 'bar')
        path = '/'.join([
            ezdiscovery.NAMESPACE,
            'foo',
            ezdiscovery.SECURITY,
            ezdiscovery.SECURITY_ID
        ])
        self.assertTrue(self.client.exists(path))
        self.assertEquals('bar', self.client.get(path)[0])

    def test_set_security_id_for_common_service(self):
        """Ensure security id's get set for common services."""
        ezdiscovery.register_common_endpoint('foo', 'localhost', 8000)
        ezdiscovery.set_security_id_for_common_service('foo', 'bar')
        path = '/'.join([
            ezdiscovery.NAMESPACE,
            '/'.join([ezdiscovery.COMMON_APP_NAME, 'foo']),
            ezdiscovery.SECURITY,
            ezdiscovery.SECURITY_ID
        ])
        self.assertTrue(self.client.exists(path))
        self.assertEquals('bar', self.client.get(path)[0])

    def test_get_security_id_for_application(self):
        """Ensure fetching application security id's returns properly."""
        # Fetch one that does not exist.
        self.assertEquals(
            None,
            ezdiscovery.get_security_id_for_application('foo')
        )
        ezdiscovery.register_endpoint('foo', 'bar', 'localhost', 8000)
        ezdiscovery.set_security_id_for_application('foo', 'bar')
        self.assertEquals(
            'bar',
            ezdiscovery.get_security_id_for_application('foo')
        )

    def test_get_security_id_for_common_service(self):
        """Ensure fetching application security id's returns properly."""
        # Fetch one does not exist.
        self.assertEquals(
            None,
            ezdiscovery.get_security_id_for_common_service('foo')
        )
        ezdiscovery.register_common_endpoint('foo', 'localhost', 8000)
        ezdiscovery.set_security_id_for_common_service('foo', 'bar')
        self.assertEquals(
            'bar',
            ezdiscovery.get_security_id_for_common_service('foo')
        )

if __name__ == '__main__':
    # You may need to modify this environment variable to locate Zookeeper.
    if 'ZOOKEEPER_PATH' not in os.environ:
        os.environ['ZOOKEEPER_PATH'] = os.sep + os.path.join(
            'home',
            getpass.getuser(),
            '.ez_dev',
            'zookeeper'
        )
    unittest.main()
