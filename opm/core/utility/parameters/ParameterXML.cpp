//===========================================================================
//
// File: ParameterXML.cpp
//
// Created: Tue Jun  2 18:57:25 2009
//
// Author(s): B�rd Skaflestad     <bard.skaflestad@sintef.no>
//            Atgeirr F Rasmussen <atgeirr@sintef.no>
//
// $Date$
//
// $Revision$
//
//===========================================================================

/*
  Copyright 2009, 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <opm/core/utility/parameters/ParameterXML.hpp>
#include <opm/core/utility/parameters/Parameter.hpp>
#include <opm/core/utility/parameters/ParameterStrings.hpp>

#include <exception>
#include <iostream>
#include <string>
#include <memory>
#include <boost/filesystem.hpp>

#include <opm/core/utility/parameters/tinyxml/tinyxml.h>

namespace Opm {
    namespace parameter {

	namespace tinyxml {
	    std::string getProperty(const std::string& property,
                                    const TiXmlElement* node_ptr);
	    void read_xml(ParameterGroup& pg, const std::string filename,
                          const bool enable_output);
	    void fill_tree(ParameterGroup& pg,
                           const TiXmlNode* root,
                           const std::string& xml_file_path,
                           const bool enable_output);
	}

	void fill_xml(ParameterGroup& pg, const std::string filename,
                      const bool enable_output) {
	    tinyxml::read_xml(pg, filename, enable_output);
	}

	namespace tinyxml {
	    std::string getProperty(const std::string& property,
                                    const TiXmlElement* node_ptr)
            {
		const char* prop_value_ptr = node_ptr->Attribute(property.c_str());
		std::string property_value(prop_value_ptr ? prop_value_ptr : "");
		return property_value;
	    }

            void read_xml(ParameterGroup& pg, const std::string filename, const bool enable_output)
            {
                TiXmlDocument doc(filename);
                bool ok = doc.LoadFile();
                if (!ok) {
		    std::cerr << "ERROR: Failed to open XML file '" << filename << "'\n";
		    throw std::exception();
                }
                const TiXmlNode* root = doc.RootElement();
		std::string xml_file_path = boost::filesystem::path(filename).branch_path().string();
		fill_tree(pg, root, xml_file_path, enable_output);
	    }

	    void fill_tree(ParameterGroup& pg,
                           const TiXmlNode* root,
                           const std::string& xml_file_path,
                           const bool enable_output)
            {
		//std::cout << "GROUP '" << value << "' BEGIN\n";
		for (const TiXmlNode* cur = root->FirstChild(); cur; cur = cur->NextSibling()) {
                    const TiXmlElement* elem = cur->ToElement();
		    if (elem) {
			std::string tag_name = elem->ValueStr();
			if (tag_name == ID_xmltag__file_params) {
			    std::string relative_filename = getProperty(ID_xmlatt__value, elem);
			    std::string filename = (boost::filesystem::path(xml_file_path) / relative_filename).string();
			    fill_xml(pg, filename, enable_output);
			    continue;
			}
			std::string name = getProperty(ID_xmlatt__name, elem);
			std::shared_ptr<ParameterMapItem> data;
			if (tag_name == ID_xmltag__param) {
			    std::string value = getProperty(ID_xmlatt__value, elem);
			    std::string type = getProperty(ID_xmlatt__type, elem);
			    if (type == ID_param_type__file) {
				value = (boost::filesystem::path(xml_file_path) / value).string();
				type  = ID_param_type__string;
			    }
			    data.reset(new Parameter(value, type));
			} else if (tag_name == ID_xmltag__param_grp) {
			    std::string child_path  = pg.path() + ID_delimiter_path + name;
			    data.reset(new ParameterGroup(child_path, &pg, enable_output));
			    fill_tree(dynamic_cast<ParameterGroup&>(*data), elem, xml_file_path,
                                      enable_output);
			} else if (tag_name == ID_xmltag__file_param_grp) {
			    std::string child_path  = pg.path() + ID_delimiter_path + name;
			    data.reset(new ParameterGroup(child_path, &pg, enable_output));
			    std::string relative_filename = getProperty(ID_xmlatt__value, elem);
			    std::string filename = (boost::filesystem::path(xml_file_path) / relative_filename).string();
			    fill_xml(dynamic_cast<ParameterGroup&>(*data), filename, enable_output);
			} else {
			    std::cerr << "ERROR: '" << tag_name << "' is an unknown xml tag.\n";
			    throw std::exception();
			}
			pg.insert(name, data);
		    }
		}
		//std::cout << "GROUP '" << id << "' END\n";
	    }
	}
    } // namespace parameter
} // namespace Opm

