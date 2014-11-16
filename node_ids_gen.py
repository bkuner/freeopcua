#!/usr/bin/python

import csv

print(
"""
/// @author Alexander Rykovanov 2014
/// @email rykovanov.as@gmail.com
/// @brief Well known attributes identifiers.
/// @license GNU LGPL
///
/// Distributed under the GNU LGPL License
/// (See accompanying file LICENSE or copy at
/// http://www.gnu.org/licenses/lgpl.html)
///

///
/// DO NOT EDIT! File is autogenerated.
///


#pragma once

#include <stdint.h>

namespace OpcUa
{
  enum class ObjectID : uint32_t
  {
    Null = 0,
"""
)

with open('NodeIds.csv', 'r') as ids_file:
	ids_reader = csv.reader(ids_file, delimiter=',')

	for row in ids_reader:
		print ("    " + row[0] + " = " + row[1] + ",")

print (
"""
    Server_ServerCapabilities_ModellingRules = 2996,
    EventTypesFolder = 3048,
    Server_ServerCapabilities_SoftwareCertificates = 3704
  };
}
"""
)