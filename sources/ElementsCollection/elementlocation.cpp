/*
		Copyright 2006-2015 The QElectroTech Team
		This file is part of QElectroTech.

		QElectroTech is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 2 of the License, or
		(at your option) any later version.

		QElectroTech is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.

		You should have received a copy of the GNU General Public License
		along with QElectroTech.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "elementlocation.h"
#include "qetapp.h"
#include "qetproject.h"
#include "elementscollectioncache.h"
#include "xmlelementcollection.h"
#include "elementfactory.h"
#include "element.h"

/**
 * @brief ElementLocation::ElementLocation
 * @param path : path of item in file system
 */
ElementLocation::ElementLocation(QString path):
	m_project(nullptr)
{
	if (!path.isEmpty())
		setPath(path);
}

/**
 * @brief ElementLocation::ElementLocation
 * @param path : path of item embedded in @project
 * The path must be in form : embed://dir/subdir/myElement.elmt
 * @param project : project
 */
ElementLocation::ElementLocation(QString path, QETProject *project) :
	m_project(project)
{
	if (!path.isEmpty())
		setPath(path);
}

ElementLocation::~ElementLocation()
{}

/**
 * @brief ElementLocation::setPath
 * Set the path of this item.
 * If the path is for a file collection, the path can be in file system or relative to the beginning
 * of the colection, in this case the path must start with (common:// or custom://).
 * @param path
 * @return true if the element pointed by path exist, else false
 */
bool ElementLocation::setPath(QString path)
{
	QString tmp_path = path;

		//There is a project, the path is for an embedded coolection.
	if (m_project)
	{
		if (path.startsWith("embed://"))
		{
			m_collection_path = path;
			return true;
		}
		else
			return false;
	}

		//The path start with project, we get the project and the path from the string
	else if (tmp_path.startsWith("project"))
	{
		QRegExp rx("^project([0-9]+)\\+(embed:\\/\\/.*)$", Qt::CaseInsensitive);
		if (rx.exactMatch(tmp_path))
		{
			bool conv_ok;
			uint project_id = rx.capturedTexts().at(1).toUInt(&conv_ok);
			if (conv_ok)
			{
				QETProject *project = QETApp::project(project_id);
				if (project)
				{
					m_collection_path = rx.capturedTexts().at(2);
					m_project = project;
					return true;
				}
			}
		}
		return false;
	}

		//The path is in file system, the given path is relative to common or custom collection
	else if (path.startsWith("common://") || path.startsWith("custom://"))
	{
		QString p;
		if (path.startsWith("common://"))
		{
			tmp_path.remove("common://");
			p = QETApp::commonElementsDir() + tmp_path;
		}
		else
		{
			tmp_path.remove("custom://");
			p = QETApp::customElementsDir() + tmp_path;
		}

			//This is an element
		if (path.endsWith(".elmt"))
		{
			QFile file(p);
			if (file.exists())
			{
				m_file_system_path = p;
				m_collection_path = path;
				return true;
			}
			return false;
		}
			//They must be a directory
		else
		{
			QDir dir(p);
			if(dir.exists())
			{
				m_file_system_path = p;
				m_collection_path = path;
				return true;
			}
			return false;
		}
	}
		//In this case, the path is supposed to be relative to the file system.
	else
	{
		if(path.endsWith(".elmt"))
		{
			QFile file(path);
			if (file.exists())
			{
				m_file_system_path = path;
				if (path.startsWith(QETApp::commonElementsDir()))
				{
					path.remove(QETApp::commonElementsDir());
					path.prepend("common://");
					m_collection_path = path;
				}
				else if (path.startsWith(QETApp::customElementsDir()))
				{
					path.remove(QETApp::customElementsDir());
					path.prepend("custom://");
					m_collection_path = path;
				}
				return true;
			}
			return false;
		}
		else
		{
			QDir dir(path);
			if (dir.exists())
			{
				m_file_system_path = path;
				if (path.startsWith(QETApp::commonElementsDir()))
				{
					path.remove(QETApp::commonElementsDir());
					path.prepend("common://");
					m_collection_path = path;
				}
				else if (path.startsWith(QETApp::customElementsDir()))
				{
					path.remove(QETApp::customElementsDir());
					path.prepend("custom://");
					m_collection_path = path;
				}
				return true;
			}
			return false;
		}
	}

	return false;
}

/**
 * @brief ElementLocation::isNull
 * @return True represent nothing
 */
bool ElementLocation::isNull() const
{
	if (!m_file_system_path.isEmpty()) return false;
	else if (!m_collection_path.isEmpty()) return false;
	else return true;
}

/**
 * @brief ElementLocation::setProject
 * @param project : set the project of this location to @project.
 */
void ElementLocation::setProject(QETProject *project) {
	m_project = project;
}

/**
 * @brief ElementLocation::isElement
 * @return true if this location represent an element
 */
bool ElementLocation::isElement() const {
	return m_collection_path.endsWith(".elmt");
}

/**
 * @brief ElementLocation::isDirectory
 * @return true if this location represent a directory
 */
bool ElementLocation::isDirectory() const {
	return !isElement();
}

/**
 * @brief ElementLocation::collectionPath
 * @return the colletion relative to the collection
 */
/**
 * @brief ElementLocation::collectionPath
 * Return the path of the represented element relative to collection
 * if @protocol is true the path is prepended by the collection type (common://, custom:// or embed://)
 * else if false, only the collection path is returned without the collection type.
 * @param protocol
 * @return the path
 */
QString ElementLocation::collectionPath(bool protocol) const
{
	if (protocol)
		return m_collection_path;
	else
	{
		QString path = m_collection_path;
		return path.remove(QRegularExpression("common://|custom://|embed://"));
	}
}

/**
 * @brief ElementLocation::fileSystemPath
 * @return The file system path of this element, (the separator is always '/' see QDir::toNativeSeparators())
 * If this element is embedded in a project return an empty string;
 */
QString ElementLocation::fileSystemPath() const
{
	if (!m_project)
		return m_file_system_path;
	else
		return QString();
}

/**
 * @brief ElementLocation::project
 * @return the project of this location if he was set.
 */
QETProject *ElementLocation::project() const {
	return m_project;
}

/**
 * @brief ElementLocation::xml
 * @return The definition of this element.
 * The definition can be null.
 */
QDomElement ElementLocation::xml()
{
	if (!m_xml.isNull())
		return m_xml;

	if (!m_project)
	{
		QFile file (m_file_system_path);
		QDomDocument docu;
		if (docu.setContent(&file))
			m_xml = docu.documentElement().cloneNode().toElement();
	}
	else
	{
		QString str = m_collection_path;
		if (isElement())
		{
			QDomElement element = m_project->embeddedElementCollection()->element(str.remove("embed://"));
			m_xml = element.firstChildElement("definition");
		}
		else
		{
			QDomElement element = m_project->embeddedElementCollection()->directory(str.remove("embed://"));
			m_xml = element;
		}
	}

	return m_xml;
}

/**
 * @brief ElementLocation::uuid
 * @return The uuid of the pointed element
 */
QUuid ElementLocation::uuid()
{
	if (!m_uuid.isNull()) return m_uuid;

		//Get the uuid of element
	QList<QDomElement>  list_ = QET::findInDomElement(xml(), "uuid");

	if (!list_.isEmpty())
		m_uuid = QUuid(list_.first().attribute("uuid"));

	return m_uuid;
}

/**
 * @brief ElementLocation::icon
 * @return The icon of the represented element.
 * If icon can't be set, return a null QIcon
 */
QIcon ElementLocation::icon()
{
	if (!m_icon.isNull()) return m_icon;

	if (!m_project)
	{
		ElementsCollectionCache *cache = QETApp::collectionCache();
		if (cache->fetchElement(*this))
			m_icon = QIcon(cache->pixmap());
	}
	else
	{
		ElementFactory *factory = ElementFactory::Instance();
		int state;
		Element *elmt = factory->createElement(*this, 0, &state);

		if (state == 0)
			m_icon = QIcon(elmt->pixmap());
	}

	return m_icon;
}

/**
 * @brief ElementLocation::name
 * @return The name of the represented element in the current local
 */
QString ElementLocation::name()
{
	if (!m_project)
	{
		ElementsCollectionCache *cache = QETApp::collectionCache();
		if (cache->fetchElement(*this))
			return cache->name();
		else
			return QString();
	}
	else
	{
		NamesList nl;
		nl.fromXml(xml());
		return nl.name(fileName());
	}
}

/**
 * @brief ElementLocation::fileName
 * @return Return the file name of this element whatever the storage system (file system, xml collection)
 */
QString ElementLocation::fileName() const
{
	if (m_collection_path.isEmpty()) return QString();

	QStringList qsl = m_collection_path.split("/");
	if (qsl.isEmpty()) return QString();
	else return qsl.last();
}