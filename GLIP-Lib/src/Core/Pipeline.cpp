/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE (ronan.kerviche@free.fr)                                                      */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : Pipeline.cpp                                                                              */
/*     Original Date : August 15th 2011                                                                          */
/*                                                                                                               */
/*     Description   : Pipeline object                                                                           */
/*                                                                                                               */
/* ************************************************************************************************************* */

/**
 * \file    Pipeline.cpp
 * \brief   Pipeline object
 * \author  R. KERVICHE
 * \date    August 15th 2011
**/

#include <limits>
#include <map>
#include "Core/Exception.hpp"
#include "Core/Pipeline.hpp"
#include "Core/Component.hpp"
#include "Core/HdlFBO.hpp"
#include "Core/ShaderSource.hpp"
#include "devDebugTools.hpp"

	using namespace Glip::CoreGL;
	using namespace Glip::CorePipeline;

// AbstractPipelineLayout
	/**
	\fn AbstractPipelineLayout::AbstractPipelineLayout(const std::string& type)
	\brief AbstractPipelineLayout constructor.
	\param type Typename of the pipeline.
	**/
	AbstractPipelineLayout::AbstractPipelineLayout(const std::string& type)
	 : AbstractComponentLayout(type)
	{ }

	/**
	\fn AbstractPipelineLayout::AbstractPipelineLayout(const AbstractPipelineLayout& c)
	\brief AbstractPipelineLayout constructor.
	\param c Copy.
	**/
	AbstractPipelineLayout::AbstractPipelineLayout(const AbstractPipelineLayout& c)
	 : AbstractComponentLayout(c)
	{
		// Copy of the whole vectors :
		elementsKind   	= c.elementsKind;
		elementsName	= c.elementsName;
		elementsID     	= c.elementsID;
		connections	= c.connections;

		for(unsigned int i=0; i<c.elementsLayout.size(); i++)
		{
			switch(elementsKind[i])
			{
				case FILTER:
					elementsLayout.push_back(reinterpret_cast<AbstractComponentLayout*>(new AbstractFilterLayout(c.filterLayout(i))));
					break;
				case PIPELINE:
					elementsLayout.push_back(reinterpret_cast<AbstractComponentLayout*>(new AbstractPipelineLayout(c.pipelineLayout(i))));
					break;
				default:
					throw Exception("AbstractPipelineLayout::AbstractPipelineLayout - Unknown type for copy for element in " + getFullName(), __FILE__, __LINE__, Exception::CoreException);
			}
		}
		//std::cout << "end copy of pipeline layout for " << getFullName() << std::endl;
	}

	AbstractPipelineLayout::~AbstractPipelineLayout(void)
	{
		for(unsigned int k=0; k<elementsLayout.size(); k++)
		{
			switch(elementsKind[k])
			{
				case FILTER:
					delete &filterLayout(k);
					break;
				case PIPELINE:
					delete &pipelineLayout(k);
					break;
				default:
					throw Exception("AbstractPipelineLayout::~AbstractPipelineLayout - Unknown type for delete for element in " + getFullName(), __FILE__, __LINE__, Exception::CoreException);
			}
		}

		connections.clear();
		elementsKind.clear();
		elementsName.clear();
		elementsID.clear();
	}

	/**
	\fn int AbstractPipelineLayout::getElementID(int i) const
	\brief Get element ID in global structure.
	\param i The ID of the element in the local pipeline layout.
	\return The ID of the element in the global structure or raise an exception if any errors occur.
	**/
	int AbstractPipelineLayout::getElementID(int i) const
	{
		checkElement(i);

		if(elementsID[i]==ELEMENT_NOT_ASSOCIATED)
			throw Exception("AbstractPipelineLayout::getElementID - Element " + toString(i) + " is not associated. Is this object part of a Pipeline?", __FILE__, __LINE__, Exception::CoreException);

		return elementsID[i];
	}

	/**
	\fn int AbstractPipelineLayout::getElementID(const std::string& name) const
	\brief Get element ID in global structure.
	\param name The ID of the element in the local pipeline layout.
	\return The ID of the element in the global structure or raise an exception if any errors occur.
	**/
	int AbstractPipelineLayout::getElementID(const std::string& name) const
	{
		return getElementID( getElementIndex(name) );
	}

	/**
	\fn void AbstractPipelineLayout::setElementID(int i, int ID)
	\brief Set element ID in global structure or raise an exception if any errors occur.
	\param i The ID of the element in the local pipeline layout.
	\param ID The ID of the element in the global structure.
	**/
	void AbstractPipelineLayout::setElementID(int i, int ID)
	{
		checkElement(i);
		elementsID[i] = ID;
	}

	/**
	\fn AbstractPipelineLayout::Connection AbstractPipelineLayout::getConnection(int i) const
	\brief Get the connection by its ID.
	\param i The ID of the connection.
	\return A copy of the corresponding Connection object or raise an exception if any errors occur.
	**/
	AbstractPipelineLayout::Connection AbstractPipelineLayout::getConnection(int i) const
	{
		if(i<0 || i>=static_cast<int>(connections.size()))
			throw Exception("AbstractPipelineLayout::getConnection - Bad connection ID for "  + getFullName() + ", ID : " + toString(i), __FILE__, __LINE__, Exception::CoreException);
		return connections[i];
	}

	/**
	\fn void AbstractPipelineLayout::checkElement(int i) const
	\brief Check if element exists and raise an exception if any errors occur.
	\param i The ID of the element.
	**/
	void AbstractPipelineLayout::checkElement(int i) const
	{
		if(i<0 || i>=static_cast<int>(elementsLayout.size()))
			throw Exception("AbstractPipelineLayout::checkElement - Bad element ID for "  + getFullName() + ", ID : " + toString(i), __FILE__, __LINE__, Exception::CoreException);
	}

	/**
	\fn int AbstractPipelineLayout::getNumElements(void) const
	\brief Get the number of elements.
	\return Number of elements.
	**/
	int AbstractPipelineLayout::getNumElements(void) const
	{
		return elementsLayout.size();
	}

	/**
	\fn int AbstractPipelineLayout::getNumConnections(void) const
	\brief Get the number of connections.
	\return Number of connections.
	**/
	int AbstractPipelineLayout::getNumConnections(void) const
	{
		return connections.size();
	}

	/**
	\fn void AbstractPipelineLayout::getInfoElements(int& numFilters, int& numPipelines, int& numUniformVariables)
	\brief Get the total number of Filters and Pipelines contained by this pipeline.
	\param numFilters The total number of filters.
	\param numPipelines The total number of pipelines (including this one).
	\param numUniformVariables APPROXIMATION to the total number of uniform variables : in some cases, the number might be larger than the actual number of uniform variables (this is because a variable can appear in both the vertex and fragment shader).
	**/
	void AbstractPipelineLayout::getInfoElements(int& numFilters, int& numPipelines, int& numUniformVariables)
	{
		int a, b, c;
		//AbstractPipelineLayout* tmp = NULL;
		numFilters  		= 0;
		numPipelines		= 0;
		numUniformVariables 	= 0;

		for(unsigned int i=0; i<elementsLayout.size(); i++)
		{
			switch(elementsKind[i])
			{
				case FILTER:
					numFilters++;
					numUniformVariables += filterLayout(i).getNumUniformVars();
					break;
				case PIPELINE:
					//tmp = reinterpret_cast<AbstractPipelineLayout*>(elementsLayout[i]);
					pipelineLayout(i).getInfoElements(a,b,c);
					numFilters   		+= a;
					numPipelines 		+= b+1;
					numUniformVariables 	+= c;
					break;
				default:
					throw Exception("AbstractPipelineLayout::getInfoElements - Unknown type for element in " + getFullName(), __FILE__, __LINE__, Exception::CoreException);
			}
		}

		numPipelines++; // include this
	}

	/**
	\fn int AbstractPipelineLayout::getElementIndex(const std::string& name) const
	\brief Get the ID of an element knowing its name.
	\param name The name of the element.
	\return The ID of the element or raise an exception if any errors occur.
	**/
	int AbstractPipelineLayout::getElementIndex(const std::string& name) const
	{
		std::vector<std::string>::const_iterator it = std::find(elementsName.begin(), elementsName.end(), name);

		if(it==elementsName.end())
			throw Exception("AbstractPipelineLayout::getElementIndex - Unable to find element \"" + name + "\" in pipeline layout " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
		else
			return std::distance(elementsName.begin(), it);
	}

	/**
	\fn bool AbstractPipelineLayout::doesElementExist(const std::string& name) const
	\brief Check if an element exists knowing its name.
	\param name The name of the element.
	\return True if such an element exists, False otherwise.
	**/
	bool AbstractPipelineLayout::doesElementExist(const std::string& name) const
	{
		std::vector<std::string>::const_iterator it = std::find(elementsName.begin(), elementsName.end(), name);

		return it!=elementsName.end();
	}

	bool AbstractPipelineLayout::doesElementExist(const std::vector<std::string>& path) const
	{
		const AbstractPipelineLayout* ptr = this;

		for(std::vector<std::string>::const_iterator it=path.begin(); it!=path.end(); it++)
		{
			if(!ptr->doesElementExist(*it))
				return false;

			int id = ptr->getElementIndex(*it);

			if(ptr->getElementKind(id)!=PIPELINE)
				return false;

			ptr = &ptr->pipelineLayout(*it);
		}

		return true;
	}

	/**
	\fn ComponentKind AbstractPipelineLayout::getElementKind(int i) const
	\brief Get the kind of an element.
	\param i The ID of the element.
	\return The kind of the element or raise an exception if any errors occur.
	**/
	ComponentKind AbstractPipelineLayout::getElementKind(int i) const
	{
		checkElement(i);
		return elementsKind[i];
	}

	/**
	\fn const std::string& AbstractPipelineLayout::getElementName(int i) const
	\brief Get the name of an element.
	\param i The name of the element.
	\return The name of the element or raise an exception if any errors occur.
	**/
	const std::string& AbstractPipelineLayout::getElementName(int i) const
	{
		checkElement(i);
		return elementsName[i];
	}

	/**
	\fn AbstractComponentLayout& AbstractPipelineLayout::componentLayout(int i) const
	\brief Get the component layout by its index.
	\param i The ID of the component.
	\return A reference to the component or raise an exception if any errors occur.
	**/
	AbstractComponentLayout& AbstractPipelineLayout::componentLayout(int i) const
	{
		checkElement(i);

		//std::cout << "ACCESSING COMPONENT (int)" << std::endl;
		//return *elementsLayout[i];
		switch(elementsKind[i])
		{
			case FILTER:
				return *reinterpret_cast<AbstractFilterLayout*>(elementsLayout[i]);
			case PIPELINE:
				return *reinterpret_cast<AbstractPipelineLayout*>(elementsLayout[i]);
			default :
				throw Exception("AbstractPipelineLayout::componentLayout - Type not recognized for element in " + getFullName() + ".",__FILE__, __LINE__, Exception::CoreException);
		}
	}

	/**
	\fn AbstractComponentLayout& AbstractPipelineLayout::componentLayout(const std::string& name) const
	\brief Get the component layout by its name.
	\param name The name of the element.
	\return A reference to the component or raise an exception if any errors occur.
	**/
	AbstractComponentLayout& AbstractPipelineLayout::componentLayout(const std::string& name) const
	{
		int index = getElementIndex(name);
		//std::cout << "ACCESSING COMPONENT (int)" << std::endl;
		return componentLayout(index);
	}

	/**
	\fn AbstractFilterLayout& AbstractPipelineLayout::filterLayout(int i) const
	\brief Get the filter layout by its index.
	\param i The ID of the filter layout.
	\return A reference to the filter layout or raise an exception if any errors occur.
	**/
	AbstractFilterLayout& AbstractPipelineLayout::filterLayout(int i) const
	{
		//std::cout << "ACCESSING FILTER (int)" << std::endl;
		checkElement(i);
		if(getElementKind(i)!=FILTER)
			throw Exception("AbstractPipelineLayout::filterLayout - The element of index " + toString(i) + " exists but is not a filter in pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
		return *reinterpret_cast<AbstractFilterLayout*>(elementsLayout[i]);
	}

	/**
	\fn AbstractFilterLayout& AbstractPipelineLayout::filterLayout(const std::string& name) const
	\brief Get the filter layout by its name.
	\param name The name of the filter layout.
	\return A reference to the filter layout or raise an exception if any errors occur.
	**/
	AbstractFilterLayout& AbstractPipelineLayout::filterLayout(const std::string& name) const
	{
		//std::cout << "ACCESSING FILTER (name)" << std::endl;
		int index = getElementIndex(name);
		if(getElementKind(index)!=FILTER)
			throw Exception("AbstractPipelineLayout::filterLayout - The element \"" + name + "\" exists but is not a filter in pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
		return *reinterpret_cast<AbstractFilterLayout*>(elementsLayout[index]);
	}

	/**
	\fn AbstractPipelineLayout& AbstractPipelineLayout::pipelineLayout(int i) const
	\brief Get the pipeline layout by its index.
	\param i The ID of the pipeline layout.
	\return A reference to the pipeline layout or raise an exception if any errors occur.
	**/
	AbstractPipelineLayout& AbstractPipelineLayout::pipelineLayout(int i) const
	{
		//std::cout << "ACCESSING PIPELINE (int)" << std::endl;
		checkElement(i);
		if(getElementKind(i)!=PIPELINE)
			throw Exception("AbstractPipelineLayout::pipelineLayout - The element of index " + toString(i) + " exists but is not a pipeline in pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
		return *reinterpret_cast<AbstractPipelineLayout*>(elementsLayout[i]);
	}

	/**
	\fn AbstractPipelineLayout& AbstractPipelineLayout::pipelineLayout(const std::string& name) const
	\brief Get the pipeline layout by its name.
	\param name The name of the pipeline layout.
	\return A reference to the pipeline layout or raise an exception if any errors occur.
	**/
	AbstractPipelineLayout& AbstractPipelineLayout::pipelineLayout(const std::string& name) const
	{
		//std::cout << "ACCESSING PIPELINE (name)" << std::endl;
		int index = getElementIndex(name);
		if(getElementKind(index)!=PIPELINE)
			throw Exception("AbstractPipelineLayout::pipelineLayout - The element \"" + name + "\" exists but is not a pipeline in pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);

		return *reinterpret_cast<AbstractPipelineLayout*>(elementsLayout[index]);
	}

	AbstractPipelineLayout& AbstractPipelineLayout::pipelineLayout(const std::vector<std::string>& path)
	{
		AbstractPipelineLayout* ptr = this;

		for(std::vector<std::string>::const_iterator it=path.begin(); it!=path.end(); it++)
			ptr = &ptr->pipelineLayout(*it);

		return *ptr;
	}

	/**
	\fn std::vector<AbstractPipelineLayout::Connection> AbstractPipelineLayout::getConnectionDestinations(int id, int p)
	\brief Get all destinations of an output port.
	\param id The ID of the output element.
	\param p The port of the output element.
	\return A vector of Connection object, all having output as (id,p) or raise an exception if any errors occur.
	**/
	std::vector<AbstractPipelineLayout::Connection> AbstractPipelineLayout::getConnectionDestinations(int id, int p)
	{
		if(id!=THIS_PIPELINE)
		{
			AbstractComponentLayout& src = componentLayout(id);
			src.checkOutputPort(p);
		}
		else
			checkInputPort(p);

		// The Element and its port exist, now find their connexions
		std::vector<Connection> result;
		for(std::vector<Connection>::iterator it=connections.begin(); it!=connections.end(); it++)
			if( (*it).idOut==id && (*it).portOut==p) result.push_back(*it);

		return result;
	}

	/**
	\fn AbstractPipelineLayout::Connection AbstractPipelineLayout::getConnectionSource(int id, int p)
	\brief Get the source of an input port.
	\param id The ID of the input element.
	\param p The port of the input element.
	\return A Connection object, having input as (id,p) or raise an exception if any errors occur.
	**/
	AbstractPipelineLayout::Connection AbstractPipelineLayout::getConnectionSource(int id, int p)
	{
		std::string str;

		if(id!=THIS_PIPELINE)
		{
			AbstractComponentLayout& src = componentLayout(id);
			src.checkInputPort(p);
		}
		else
			checkOutputPort(p);

		// The Element and its port exist, now find the connexion
		for(std::vector<Connection>::iterator it=connections.begin(); it!=connections.end(); it++)
			if( (*it).idIn==id && (*it).portIn==p) return (*it);

		if(id!=THIS_PIPELINE)
		{
			AbstractComponentLayout& src = componentLayout(id);
			throw Exception("Element \"" + getElementName(id) + "\" (typename : \"" + src.getTypeName() + "\") has no source on input port \"" + src.getInputPortName(p) + "\" (id : " + toString(p) + ") in pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
		}
		else
			throw Exception("This Pipeline " + getFullName() + " has no source on output port \"" + getOutputPortName(p) + "\" (id : " + toString(p) + ").", __FILE__, __LINE__, Exception::CoreException);
	}

	/**
	\fn bool AbstractPipelineLayout::check(bool exception)
	\brief Check the validity of the pipeline layout.
	\param exception If set to true, an exception would be raised if any error is found.
	\return true if valid, false otherwise.
	**/
	bool AbstractPipelineLayout::check(bool exception)
	{
		std::string res;

		for(unsigned int i=0; i<elementsLayout.size(); i++)
		{
			AbstractComponentLayout& tmp = componentLayout(i);

			for(int j=0; j<tmp.getNumInputPort(); j++)
			{
				try
				{
					getConnectionSource(i, j);
				}
				catch(Exception& e)
				{
					res += '\n';
					res += e.getMessage();
				}
			}

			// Checking that, at least one connection is used : 
			bool test = false;
			for(int j=0; j<tmp.getNumOutputPort(); j++)
			{
				if(!getConnectionDestinations(i,j).empty())
				{
					test = true;
					break;
				}
			}

			if(!test)
			{
				res += '\n';
				res += "Element \"" + getElementName(i) + "\" (type : \"" + tmp.getTypeName() + "\") has no output port connected.";
			}	
		}

		for(int i=0; i<getNumOutputPort(); i++)
		{
			try
			{
				getConnectionSource(THIS_PIPELINE, i);
			}
			catch(Exception& e)
			{
				res += '\n';
				res += e.getMessage();
			}
		}

		for(int i=0; i<getNumInputPort(); i++)
		{
			if(getConnectionDestinations(THIS_PIPELINE,i).empty())
			{
				res += '\n';
				res += "Input port \"" + getInputPortName(i) + "\" is not connected inside the pipeline.";
			}
		}

		if(exception && !res.empty())
			throw Exception("PipelineLayout::check - The following errors has been found in the PipelineLayout " + getFullName() + " : " + res, __FILE__, __LINE__, Exception::CoreException);
		else if(!res.empty())
			return false;

		return true;
	}

// PipelineLayout
	/**
	\fn PipelineLayout::PipelineLayout(const std::string& type)
	\brief PipelineLayout constructor.
	\param type The typename.
	**/
	PipelineLayout::PipelineLayout(const std::string& type)
	 : 	AbstractComponentLayout(type), 
		ComponentLayout(type), 
		AbstractPipelineLayout(type)
	{ }

	/**
	\fn PipelineLayout::PipelineLayout(const AbstractPipelineLayout& c)
	\brief PipelineLayout constructor.
	\param c Copy.
	**/
	PipelineLayout::PipelineLayout(const AbstractPipelineLayout& c)
	 : 	AbstractComponentLayout(c), 
		ComponentLayout(c),
		AbstractPipelineLayout(c)
	{ }

	/**
	\fn int PipelineLayout::add(const AbstractFilterLayout& filterLayout, const std::string& name)
	\brief Add a filter to the pipeline layout.
	\param filterLayout The filter layout.
	\param name The name of the element.
	\return The ID of the element added.
	**/
	int PipelineLayout::add(const AbstractFilterLayout& filterLayout, const std::string& name)
	{
		if(doesElementExist(name))
			throw Exception("PipelineLayout::add - An element with the name " + name + " already exists in pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);

		AbstractFilterLayout* tmp = new AbstractFilterLayout(filterLayout);
		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "PipelineLayout::add : <" << name << '>' << std::endl;
		#endif
		elementsLayout.push_back(reinterpret_cast<AbstractComponentLayout*>(tmp));
		elementsKind.push_back(FILTER);
		elementsName.push_back(name);
		elementsID.push_back(ELEMENT_NOT_ASSOCIATED);
		return elementsLayout.size()-1;
	}

	/**
	\fn int PipelineLayout::add(const AbstractPipelineLayout& pipelineLayout, const std::string& name)
	\brief Add a subpipeline to the pipeline layout.
	\param pipelineLayout The pipeline layout.
	\param name The name of the element.
	\return The ID of the element added.
	**/
	int PipelineLayout::add(const AbstractPipelineLayout& pipelineLayout, const std::string& name)
	{
		if(doesElementExist(name))
			throw Exception("PipelineLayout::add - An element with the name " + name + " already exists in pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);

		AbstractPipelineLayout* tmp = new AbstractPipelineLayout(pipelineLayout);
		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "PipelineLayout::add : <" << name << '>' << std::endl;
		#endif
		elementsLayout.push_back(reinterpret_cast<AbstractComponentLayout*>(tmp));
		elementsKind.push_back(PIPELINE);
		elementsName.push_back(name);
		elementsID.push_back(ELEMENT_NOT_ASSOCIATED);
		return elementsLayout.size()-1;
	}

	/**
	\fn int PipelineLayout::addInput(const std::string& name)
	\brief Add an input port to the pipeline layout.
	\param name The name of the new input port.
	\return The ID of the new input port.
	**/
	int PipelineLayout::addInput(const std::string& name)
	{
		return addInputPort(name);
	}

	/**
	\fn int PipelineLayout::addOutput(const std::string& name)
	\brief Add an output port to the pipeline layout.
	\param name The name of the new output port.
	\return The ID of the new output port.
	**/
	int PipelineLayout::addOutput(const std::string& name)
	{
		return addOutputPort(name);
	}

	/**
	\fn void PipelineLayout::connect(int filterOut, int portOut, int filterIn, int portIn)
	\brief Create a connection between two elements or an element and this pipeline and raise an exception if any errors occur.
	\param filterOut The ID of the output element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portOut The ID of the output port.
	\param filterIn The ID of the input element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portIn The ID of the input port.
	**/
	void PipelineLayout::connect(int filterOut, int portOut, int filterIn, int portIn)
	{
		if(filterOut==THIS_PIPELINE && filterIn==THIS_PIPELINE)
			throw Exception("PipelineLayout::connect - can't connect directly an input to an output in pipeline " + getFullName() + ", you don't need that.", __FILE__, __LINE__, Exception::CoreException);

		if(filterOut!=THIS_PIPELINE)
		{
			AbstractComponentLayout& fo = componentLayout(filterOut); // Source
			fo.checkOutputPort(portOut);
		}
		else
			checkInputPort(portOut);

		if(filterIn!=THIS_PIPELINE)
		{
			AbstractComponentLayout& fi = componentLayout(filterIn);  // Destination
			fi.checkInputPort(portIn);
		}
		else
			checkOutputPort(portIn);

		// Check if a connexion already exist to the destination :
		for(std::vector<Connection>::iterator it=connections.begin(); it!=connections.end(); it++)
			if( (*it).idIn==filterIn && (*it).portIn==portIn)
			{
				if(filterIn!=THIS_PIPELINE)
				{
					throw Exception("PipelineLayout::connect - A connexion already exists to the destination : " + componentLayout(filterIn).getFullName() + " on port " + componentLayout(filterIn).getInputPortName(portIn) + " in pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
				}
				else
				{
					throw Exception("PipelineLayout::connect - A connexion already exists to this pipeline output : " + getFullName() + " on port " + getInputPortName(portIn) + " in pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
				}
			}

		Connection c;
		c.idOut   = filterOut;
		c.portOut = portOut;
		c.idIn    = filterIn;
		c.portIn  = portIn;

		connections.push_back(c);
	}

	/**
	\fn void PipelineLayout::connect(const std::string& filterOut, const std::string& portOut, const std::string& filterIn, const std::string& portIn)
	\brief Create a connection between two elements or an element and this pipeline and raise an exception if any errors occur.
	\param filterOut The name of the output element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portOut The name of the output port.
	\param filterIn The name of the input element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portIn The name of the input port.
	**/
	void PipelineLayout::connect(const std::string& filterOut, const std::string& portOut, const std::string& filterIn, const std::string& portIn)
	{
		int fi = getElementIndex(filterIn),
		fo = getElementIndex(filterOut),
		pi = componentLayout(filterIn).getInputPortID(portIn),
		po = componentLayout(filterOut).getOutputPortID(portOut);

		connect(fo, po, fi, pi); // Check-in done twice but...
	}

	/**
	\fn void PipelineLayout::connectToInput(int port, int filterIn,  int portIn)
	\brief Create a connection between an input port of this pipeline and one of its element and raise an exception if any errors occur.
	\param port The ID of the port for this pipeline.
	\param filterIn The ID of the input element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portIn The ID of the input port.
	**/
	void PipelineLayout::connectToInput(int port, int filterIn,  int portIn)
	{
		connect(THIS_PIPELINE, port, filterIn, portIn);
	}

	/**
	void PipelineLayout::connectToInput(const std::string& port, const std::string& filterIn, const std::string& portIn)
	\brief Create a connection between an input port of this pipeline and one of its element and raise an exception if any errors occur.
	\param port The name of the port for this pipeline.
	\param filterIn The name of the input element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portIn The name of the input port.
	**/
	void PipelineLayout::connectToInput(const std::string& port, const std::string& filterIn, const std::string& portIn)
	{
		try
		{
			int p  = getInputPortID(port),
			fi = getElementIndex(filterIn),
			pi = componentLayout(filterIn).getInputPortID(portIn);
			connect(THIS_PIPELINE, p, fi, pi);
		}
		catch(Exception& e)
		{
			Exception m("PipelineLayout::connectToInput (str) - Caught an exception for the object " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}
		catch(std::exception& e)
		{
			Exception m("PipelineLayout::connectToInput (str) - Caught an exception for the object " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}
	}

	/**
	\fn void PipelineLayout::connectToOutput(int filterOut, int portOut, int port)
	\brief Create a connection between an output port of this pipeline and one of its element and raise an exception if any errors occur.
	\param filterOut The ID of the output element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portOut The ID of the output port.
	\param port The ID of the port for this pipeline.
	**/
	void PipelineLayout::connectToOutput(int filterOut, int portOut, int port)
	{
		connect(filterOut, portOut, THIS_PIPELINE, port);
	}

	/**
	\fn void PipelineLayout::connectToOutput(const std::string& filterOut, const std::string& portOut, const std::string& port)
	\brief Create a connection between an output port of this pipeline and one of its element and raise an exception if any errors occur.
	\param filterOut The name of the output element (a FilterLayout, a PipelineLayout or THIS_PIPELINE).
	\param portOut The name of the output port.
	\param port The name of the port for this pipeline.
	**/
	void PipelineLayout::connectToOutput(const std::string& filterOut, const std::string& portOut, const std::string& port)
	{
		try
		{
			int p  = getOutputPortID(port),
			fo = getElementIndex(filterOut),
			po = componentLayout(filterOut).getOutputPortID(portOut);
			connect(fo, po, THIS_PIPELINE, p);
		}
		catch(Exception& e)
		{
			Exception m("PipelineLayout::connectToOutput (str) - Caught an exception for the object " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}
		catch(std::exception& e)
		{
			Exception m("PipelineLayout::connectToOutput (str) - Caught an exception for the object " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}
	}

	/**
	\fn void PipelineLayout::autoConnect(void)
	\brief Create automatically all connections based on the name of the input and output ports and the name of the input and output textures of all Filters.

	This function will automatically make the connection based on the name of the input and output ports and the name of the different textures in the shaders used.

	All of the names of the output textures (defined with <i>out vec4</i> in the fragment shaders) and the input ports of this pipeline <b>must be unique</b>, it will raise an exception otherwise.
	**/
	void PipelineLayout::autoConnect(void)
	{
		try
		{
			// Check for previous existing connections
			if(getNumConnections()!=0)
				throw Exception("PipelineLayout::autoConnect - Layout for " + getFullName() + " has already connections and, thus, it is not eligible to auto-connect.", __FILE__, __LINE__, Exception::CoreException);

			// Check for double names in outputs :
			std::map<std::string,int> outputNames;

			// Push the inputs of this :
			for(int i=0; i<getNumInputPort(); i++)
				outputNames[getInputPortName(i)] = THIS_PIPELINE;

			// Add all of the outputs of all sub components :
			for(int i=0; i<getNumElements(); i++)
			{
				AbstractComponentLayout& cp = componentLayout(i);

				for(int j=0; j<cp.getNumOutputPort(); j++)
				{
					std::string name = cp.getOutputPortName(j);

					// Check for doubles :
					std::map<std::string,int>::iterator it = outputNames.find(name);
					if(it!=outputNames.end())
						throw Exception("PipelineLayout::autoConnect - Found another output having the same name \"" + name + "\" for PipelineLayout " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
					else
						outputNames[name] = i;
				}
			}

			// Layout is clean : start auto-connect
			for(int i=0; i<getNumElements(); i++)
			{
				AbstractComponentLayout& cp = componentLayout(i);
				for(int j=0; j<cp.getNumInputPort(); j++)
				{
					std::map<std::string,int>::iterator it = outputNames.find(cp.getInputPortName(j));

					if(it==outputNames.end())
						throw Exception("PipelineLayout::autoConnect - No elements were found having an output named \"" + cp.getInputPortName(j) + "\" for PipelineLayout " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);

					if(it->second!=THIS_PIPELINE)
						connect(getElementName(it->second), it->first, getElementName(i), cp.getInputPortName(j));
					else
						connectToInput(it->first, getElementName(i), cp.getInputPortName(j));
				}
			}

			// Connect output ports :
			for(int i=0; i<getNumOutputPort(); i++)
			{
				std::map<std::string,int>::iterator it = outputNames.find(getOutputPortName(i));

				if(it==outputNames.end())
					throw Exception("PipelineLayout::autoConnect - No elements were found having an output named \"" +getOutputPortName(i) + "\" for PipelineLayout " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);

				if(it->second!=THIS_PIPELINE)
					connectToOutput(getElementName(it->second), it->first, getOutputPortName(i));
				else
					throw Exception("PipelineLayout::autoConnect - can't connect directly an input to an output in pipeline " + getFullName() + ", you don't need that.", __FILE__, __LINE__, Exception::CoreException);
			}
		}
		catch(Exception& e)
		{
			Exception m("PipelineLayout::autoConnect - An error occured while building connection for " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}
	}

// Pipeline::BufferFormatsCell
	int Pipeline::BufferFormatsCell::size(void) const
	{
		return formats.size();
	}

	void Pipeline::BufferFormatsCell::append(const HdlAbstractTextureFormat& fmt, int count)
	{
		formats.push_back(fmt);
		outputCounts.push_back(count);
	}

// Pipeline::BuffersCell
	Pipeline::BuffersCell::BuffersCell(const BufferFormatsCell& bufferFormats)
	{
		for(unsigned int k=0; k<bufferFormats.formats.size(); k++)
			buffersList.push_back( new HdlFBO(bufferFormats.formats[k], bufferFormats.outputCounts[k]) );
	}

	Pipeline::BuffersCell::~BuffersCell(void)
	{
		for(std::vector<HdlFBO*>::iterator it = buffersList.begin(); it!=buffersList.end(); it++)
			delete (*it);
		buffersList.clear();
	}

// Pipeline
	/**
	\fn Pipeline::Pipeline(const AbstractPipelineLayout& p, const std::string& name, bool fake)
	\brief NODESC.
	**/
	Pipeline::Pipeline(const AbstractPipelineLayout& p, const std::string& name, bool fake)
	 : 	AbstractComponentLayout(p), 
		AbstractPipelineLayout(p), 
		Component(p, "(Intermediate : " + name + ")"), 
		currentCell(NULL),
		perfsMonitoring(false), 	
		queryObject(0)
	{
		UNUSED_PARAMETER(fake)
	}

	/**
	\fn Pipeline::Pipeline(const AbstractPipelineLayout& p, const std::string& name)
	\brief Pipeline constructor.
	\param p Pipeline layout.
	\param name Name of the pipeline.
	**/
	Pipeline::Pipeline(const AbstractPipelineLayout& p, const std::string& name)
	 :	AbstractComponentLayout(p), 
		AbstractPipelineLayout(p), 
		Component(p, name),
		currentCell(NULL), 
		perfsMonitoring(false), 
		queryObject(0) 
	{
		cleanInput();

		firstRun 	= true;
		broken		= true; // Wait for complete initialization.

		try
		{
			std::vector<Connection> connections;
			int idx = THIS_PIPELINE;
			build(idx, filtersList, filtersGlobalIDsList, connections, *this);
			allocateBuffers(connections);
		}
		catch(Exception& e)
		{
			//std::cout << "in Pipeline : " << std::endl << e.what() << std::endl;
			Exception m("Exception caught while building Pipeline " + getFullName() + " : ", __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}

		broken		= false;
	}

	Pipeline::~Pipeline(void)
	{
		cleanInput();

		currentCell = NULL;
		for(std::map<int, BuffersCell*>::iterator it=cells.begin(); it!=cells.end(); it++)
		{
			delete it->second;
			it->second = NULL;
		}		
		cells.clear();

		for(std::vector<Filter*>::iterator it = filtersList.begin(); it!=filtersList.end(); it++)
			delete (*it);
		filtersList.clear();

		if(queryObject>0)
			glDeleteQueries(1, &queryObject);
	}

	/**
	\fn void Pipeline::cleanInput(void)
	\brief Clean all inputs from previously acquired texture pointers.
	**/
	void Pipeline::cleanInput(void)
	{
		inputsList.clear();
	}

	void Pipeline::build(int& currentIdx, std::vector<Filter*>& filters, std::map<int, int>& filtersGlobalID, std::vector<Connection>& connections, AbstractPipelineLayout& originalLayout)
	{
		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "BUILD" << std::endl;
		#endif

		const int thisPipelineIdx = currentIdx;
		currentIdx++;

		// Extract and build elements, get the connections done too :
		try
		{
			// First, test the basic connections :
			check(true);

			std::vector<int> 	localToGlobalIdx;
			std::vector<Connection> localConnections;

			// First extract the elements :
			for(int k=0; k<getNumElements(); k++)
			{
				ComponentKind currentElKind = getElementKind(k);

				if(currentElKind==FILTER)
				{
					#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
						std::cout << "    Adding a new filter" << std::endl;
					#endif
					originalLayout.setElementID(k, currentIdx);
					localToGlobalIdx.push_back(currentIdx);

					filters.push_back(new Filter(filterLayout(k), getElementName(k)));

					// Save the link to the global ID :
					filtersGlobalID[currentIdx] = filters.size()-1;

					currentIdx++;
					#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
						std::cout << "    Adding : " << filters.back()->getFullName() << std::endl;
						std::cout << "    ID     : " << originalLayout.getElementID(k) << std::endl;
					#endif
				}
				else if(currentElKind==PIPELINE)
				{
					#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
						std::cout << "    Adding a new Pipeline" << std::endl;
					#endif
					originalLayout.setElementID(k, currentIdx);
					localToGlobalIdx.push_back(currentIdx);

					// Create a sub-pipeline :
					Pipeline tmpPipeline( pipelineLayout(k), getElementName(k), false);
					tmpPipeline.build(currentIdx, filters, filtersGlobalID, localConnections, pipelineLayout(k));

					currentIdx++;
					#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
						std::cout << "    Adding : " << pipelineLayout(k).getFullName() << std::endl;
						std::cout << "    ID     : " << originalLayout.getElementID(k) << std::endl;
					#endif
				}
				else
					throw Exception("Unknown element kind.", __FILE__, __LINE__, Exception::CoreException);
			}

			// Then change the connections :
			// The main rule is : each pipeline scale MUST untangle and shorten all of ITS connection before passing to the upper level.
			std::vector<Connection> innerOutputConnections;
			for(int k=0; k<getNumConnections(); k++)
			{
				bool saveConnection = true;
				Connection c = getConnection(k);

				// If the output is this pipeline (an input port) :
				if(c.idOut==THIS_PIPELINE)
					c.idOut = thisPipelineIdx;
				else if(getElementKind(c.idOut)==PIPELINE) // or is a pipeline...
				{
					const int elIdx = localToGlobalIdx[c.idOut];

					// We have identified a connection for which the source is actually the output of a pipeline.
					// This means that this connection is either already identified in innerOutputConnections
					// (and thus removed from localConections) or that we have to find first.

					int idx = -1;
					for(unsigned int l=0; l<innerOutputConnections.size(); l++)
					{
						if(innerOutputConnections[l].idIn==elIdx && innerOutputConnections[l].portIn==c.portOut)
						{
							idx = l;
							break;
						}
					}

					// Otherwise, try to find it in localConnections and push it in innerOutputConnections :
					if(idx<0)
					{
						for(unsigned int l=0; l<localConnections.size(); l++)
						{
							if(localConnections[l].idIn==elIdx && localConnections[l].portIn==c.portOut)
							{
								idx = innerOutputConnections.size();
								innerOutputConnections.push_back( localConnections[l] );
								localConnections.erase( localConnections.begin()+l );		// We know the nature of this connection now, and we won't use it anymore in localConnections
								break;
							}
						}
					}

					// Manage a possible error :
					if(idx<0)
						throw Exception("Unable to find interior connection to element " + componentLayout(c.idOut).getFullName() + ", port : " + toString(c.portOut) + ".", __FILE__, __LINE__, Exception::CoreException);

					// Finally : shorten the connection :
					c.idOut = innerOutputConnections[idx].idOut;
					c.portOut = innerOutputConnections[idx].portOut;
				}
				else // otherwise...
					c.idOut = localToGlobalIdx[c.idOut];

				// If the input is this pipeline :
				if(c.idIn==THIS_PIPELINE)
					c.idIn = thisPipelineIdx;
				else if(getElementKind(c.idIn)==PIPELINE) // or is a pipeline...
				{
					const int elIdx = localToGlobalIdx[c.idIn];

					// Shorten paths :
					for(unsigned int l=0; l<localConnections.size(); l++)
					{
						if(localConnections[l].idOut==elIdx && localConnections[l].portOut==c.portIn) // If the connections correspond to the other end.
						{
							localConnections[l].idOut = c.idOut;
							localConnections[l].portOut = c.portOut;
						}
					}

					saveConnection = false;
				}
				else // otherwise...
					c.idIn = localToGlobalIdx[c.idIn];

				// Save connection :
				if(saveConnection)
					connections.push_back(c);
			}

			// Sanitize the localConnections : 
			if(thisPipelineIdx==THIS_PIPELINE)
			{
				// This is made only at the tope level.
				// The main goal is to find remaining connections which are connected to the output port of a sub-pipeline
				// but have prolongation. They must be forgotten (the data will not be used).

				for(std::vector<Connection>::iterator it = localConnections.begin(); it!=localConnections.end(); )
				{
					// If the connection input is not a registered filter, then remove (C++03) : 
					if(filtersGlobalID.find(it->idIn)==filtersGlobalID.end())
					{
						std::vector<Connection>::iterator e = it;
						it++;
						localConnections.erase(e);
					}
					else
						it++;
				}
			}

			// Finally, append to the connections list :
			connections.insert(connections.end(), localConnections.begin(), localConnections.end());
		}
		catch(Exception& e)
		{
			Exception m("Pipeline::build - Error while building the pipeline " + getFullName() + " : ", __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}
		catch(std::exception& e)
		{
			Exception m("Pipeline::build - Error (std) while building the pipeline " + getFullName() + " : ", __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}

		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "END BUILD" << std::endl;
		#endif
	}

	void Pipeline::allocateBuffers(std::vector<Connection>& connections)
	{
		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "ALLOCATE" << std::endl;

			std::cout << "    Connections list : " << std::endl;
			for(std::vector<Connection>::const_iterator it=connections.begin(); it!=connections.end(); it++)
			{
				std::cout << "        From " << it->idOut << "::" << it->portOut << " to " << it->idIn << "::" << it->portIn << std::endl;

				std::string 	outElement, 
						outPort,
						inElement,
						inPort;

				if(it->idOut==THIS_PIPELINE) // An input port of this pipeline
				{
					outElement = "<THIS:" + getName() + ">";
					outPort = Component::getInputPortName(it->portOut);
				}
				else
				{
					outElement = filtersList[filtersGlobalIDsList[it->idOut]]->getName();
					outPort = filtersList[filtersGlobalIDsList[it->idOut]]->getOutputPortName(it->portOut);
				}

				if(it->idIn==THIS_PIPELINE) // An output port of this pipeline
				{
					inElement = "<THIS:" + getName() + ">";
					inPort = Component::getOutputPortName(it->portIn);
				}
				else
				{
					inElement = filtersList[filtersGlobalIDsList[it->idIn]]->getName();
					inPort = filtersList[filtersGlobalIDsList[it->idIn]]->getInputPortName(it->portIn);
				}

				std::cout << "            > " << outElement << "::" << outPort << " to " << inElement << "::" << inPort << std::endl;
			}
			std::cout << "    End connections list." << std::endl;
		#endif

		try
		{
			// The input is a list of all the connections, untangle, where the ID -1 is reserved for this pipeline.
			std::vector<Connection> 	remainingConnections = connections;	// The remaining (not processed) connections.
			std::vector<int>		requestedInputConnections;		// The number of connections not satisfied for this filter.
			std::vector<int>		bufferOccupancy;			// The number of filter which will use the buffer as inputs.
			std::vector<ActionHub>		tmpActions;				// The temporary actions list.
			bool allProcessed = false;

			// Initialize the outputs :
			OutputHub blankOutput;
			blankOutput.bufferIdx = -1;
			blankOutput.outputIdx = -1;
			outputsList.assign( getNumOutputPort(), blankOutput );

			// Setup the requirements counters :
			for(unsigned int k=0; k<filtersList.size(); k++)
			{
				ActionHub hub;

				hub.inputBufferIdx.assign( filtersList[k]->getNumInputPort(), -1);
				hub.inputArgumentIdx.assign( filtersList[k]->getNumInputPort(), -1);
				hub.bufferIdx		= -1;
				hub.filterIdx 		= k;

				tmpActions.push_back(hub);

				// Set the number of inputs not satisfied to be equal to the number of inputs :
				requestedInputConnections.push_back( filtersList[k]->getNumInputPort() );
			}

			// Initialize by decrementing the connections to this pipeline inputs :
			for(unsigned int k=0; k<remainingConnections.size(); k++)
			{
				if(remainingConnections[k].idOut==THIS_PIPELINE)
				{
					// Set up the links :
					const int fid = filtersGlobalIDsList[remainingConnections[k].idIn];

					tmpActions[fid].inputBufferIdx[ remainingConnections[k].portIn ] 	= THIS_PIPELINE;
					tmpActions[fid].inputArgumentIdx[ remainingConnections[k].portIn ]	= remainingConnections[k].portOut;
					requestedInputConnections[fid]--;

					// Remove :
					remainingConnections.erase( remainingConnections.begin() + k );
					k--;
				}
			}

			// Generate the final actions list and buffers :
			do
			{

				#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
					std::cout << "    Finding candidates : " << std::endl;
				#endif
				// Explore the list to find the possibles solutions :
				std::vector<int> candidatesIdx;
				for(unsigned int k=0; k<requestedInputConnections.size(); k++)
				{
					if(requestedInputConnections[k]==0)
					{
						candidatesIdx.push_back(k);
						#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
							std::cout << "        Adding : " << filtersList[k]->getFullName() << std::endl;
						#endif
					}
				}

				if(candidatesIdx.empty())
					throw Exception("No available filter matches input conditions.", __FILE__, __LINE__, Exception::CoreException);

				// Find best candidate along the priority order (from highest to lowest) :
				// 1 - A filter that has exactly the same format as a buffer which is currently unused, this buffer must as large as possible.
				// 2 - A filter that has the largest format which is not allocated yet.
				// 3 - A filter that has the smallest format already allocated (but not available).
				// 4 - (reserved)
				// 5 - No decision.

				HdlTextureFormat fmt(0,0,GL_RGB,GL_UNSIGNED_BYTE);	// A format (in case we need to create one).
				int fIdx = -1;						// The index of the filter chosen.
				int bIdx = -1;						// The buffer index (if one already exists).
				int currentDecision = 5;				// No decision.

				for(unsigned int k=0; k<candidatesIdx.size(); k++)
				{
					// Try to find a match in the buffers :
					bool noMatch = true;
					for(int l=0; l<bufferFormats.size(); l++)
					{
						// If this buffer is a match :
						if( *filtersList[ candidatesIdx[k] ] == bufferFormats.formats[l] )
						{
							noMatch = false;

							// If this buffer is free :
							if(bufferOccupancy[l]==0)
							{
								// If the current choice is lower in priority or larger in buffer size :
								if( currentDecision>1 || ((currentDecision==1) && fmt.getSize()<bufferFormats.formats[l].getSize()) )
								{
									fIdx = candidatesIdx[k];
									bIdx = l;
									currentDecision = 1;
									fmt = bufferFormats.formats[l];
								}
							}
							else // The buffer is not free :
							{
								// If no other choices were made :
								if(currentDecision>3 || ( (currentDecision==3) && (fmt.getSize()<bufferFormats.formats[l].getSize()) ) )
								{
									fIdx = candidatesIdx[k];
									bIdx = -1;
									currentDecision = 3;
									fmt = bufferFormats.formats[l];
								}
							}
						}
					}

					if(noMatch && currentDecision>=2 && filtersList[ candidatesIdx[k] ]->getSize() > fmt.getSize())
					{
						fIdx = candidatesIdx[k];
						bIdx = -1;
						currentDecision = 2;
						fmt = *filtersList[ candidatesIdx[k] ];
					}
				}

				#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
					std::cout << "    Decision : " << currentDecision << std::endl;
					std::cout << "    Filter   : " << filtersList[fIdx]->getFullName() << std::endl;
					std::cout << "    Buffer   : " << bIdx << std::endl;
				#endif

				if(currentDecision>=5)
					throw Exception("No correct decision was made.", __FILE__, __LINE__, Exception::CoreException);

				// Create the action :
				if(currentDecision==1)
				{
					tmpActions[ fIdx ].bufferIdx = bIdx;

					// Push :
					actionsList.push_back( tmpActions[ fIdx ] );
				}
				else if(currentDecision==2 || currentDecision==3)
				{
					// Create a new buffer :
					bufferFormats.append( fmt, filtersList[ fIdx ]->getNumOutputPort() );
					bufferOccupancy.push_back(0);

					bIdx = bufferFormats.size()-1;
					tmpActions[ fIdx ].bufferIdx = bIdx;

					// Push :
					actionsList.push_back( tmpActions[ fIdx ] );
				}
				else
					throw Exception("Internal error : Unkown action decision (" + toString(currentDecision) + ").", __FILE__, __LINE__, Exception::CoreException);

				// Find all the buffers read by the current actions and decrease their occupancy :
				for(unsigned int k=0; k<actionsList.back().inputBufferIdx.size(); k++)
				{
					if( actionsList.back().inputBufferIdx[k]!=THIS_PIPELINE )
						bufferOccupancy[ actionsList.back().inputBufferIdx[k] ]--;
				}

				// Lock down this filter as "done" :
				requestedInputConnections[fIdx] = -1;

				// Update the buffer occupancy to the new number of output and at the same time decrease the number of requests :
				for(unsigned int k=0; k<remainingConnections.size(); k++)
				{
					if(filtersGlobalIDsList[remainingConnections[k].idOut]==fIdx)
					{
						if(remainingConnections[k].idIn==THIS_PIPELINE)
						{
							outputsList[ remainingConnections[k].portIn ].bufferIdx 	= bIdx;
							outputsList[ remainingConnections[k].portIn ].outputIdx		= remainingConnections[k].portOut;

							bufferOccupancy[bIdx] = std::numeric_limits<int>::max();
						}
						else
						{
							// Set up the links :
							const int fid = filtersGlobalIDsList[remainingConnections[k].idIn];

							tmpActions[fid].inputBufferIdx[ remainingConnections[k].portIn ] 	= bIdx;
							tmpActions[fid].inputArgumentIdx[ remainingConnections[k].portIn ]	= remainingConnections[k].portOut;
							requestedInputConnections[fid]--;

							// Remove :
							remainingConnections.erase( remainingConnections.begin() + k );
							k--;

							bufferOccupancy[bIdx]++;
						}
					}
				}

				// Test if all were processed :
				allProcessed 	= true;
				bool noNextStep = true;
				for(unsigned int k=0; k<requestedInputConnections.size(); k++)
				{
					if(requestedInputConnections[k]>=0)
						allProcessed = false;
					if(requestedInputConnections[k]==0)
						noNextStep = false;
				}

				if(noNextStep && !allProcessed)
					throw Exception("The pipeline building process is stuck as some elements remains but cannot be integrated as they lack input.", __FILE__, __LINE__, Exception::CoreException);
			}
			while(!allProcessed);

			// Final tests :
			if(filtersList.size()!=actionsList.size())
				throw Exception("Some filters were omitted because their connections scheme does not allow usage.", __FILE__, __LINE__, Exception::CoreException);

		}
		catch(Exception& e)
		{
			Exception m("Pipeline::allocateBuffers - Error while allocating the buffers in the pipeline " + getFullName() + " : ", __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}
		catch(std::exception& e)
		{
			Exception m("Pipeline::allocateBuffers - Error (std) while allocating the buffers in the pipeline " + getFullName() + " : ", __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}

		// Create the first cell : 
		int cellID = createBuffersCell();

		// Link it : 
		changeTargetBuffersCell(cellID);

		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "END ALLOCATE" << std::endl;
		#endif
	}

	/**
	\fn int Pipeline::getNumActions(void) const
	\brief Get the number of filters applied during processing.
	\return Number of filters applied during processing.
	**/
	int Pipeline::getNumActions(void) const
	{
		return actionsList.size();
	}

	/**
	\fn int Pipeline::getSize(bool askDriver)
	\brief Get the size in bytes of the elements on the GPU for this pipeline.
	\param  askDriver If true, it will use HdlTexture::getSizeOnGPU() to determine the real size (might be slower).
	\return Size in bytes.
	**/
	int Pipeline::getSize(bool askDriver)
	{
		int size = 0;

		#ifdef __GLIPLIB_VERBOSE__
			std::cout << "Pipeline::getSize for " << getFullName() << " (" << bufferFormats.size() << " buffers per cell, " << cells.size() << "cells total)" <<std::endl;
		#endif

		if(askDriver && currentCell==NULL)
			throw Exception("Pipeline::getSize - Unable to get the sizes of the GL objects from the Driver as no cell is currently in use.", __FILE__, __LINE__, Exception::CoreException);

		for(int i=0; i<bufferFormats.size(); i++)
		{
			int fsize = 0;
	
			if(askDriver)
				fsize = currentCell->buffersList[i]->getSize(askDriver);
			else
				fsize = bufferFormats.formats[i].getSize();

			#ifdef __GLIPLIB_VERBOSE__
				std::cout << "    - Buffer " << i << " : " << fsize/(1024.0*1024.0) << "MB (W:" << bufferFormats.formats[i].getWidth() << ", H:" << bufferFormats.formats[i].getHeight() << ",T:" << bufferFormats.outputCounts[i] << ')' << std::endl;
			#endif
			size += fsize;
		}

		#ifdef __GLIPLIB_VERBOSE__
			std::cout << "Pipeline::getSize END" << std::endl;
		#endif

		return size;
	}

	/**
	\fn void Pipeline::process(void)
	\brief Apply the pipeline.
	**/
	void Pipeline::process(void)
	{
		clock_t 	timing 		= 0,
				totalTiming 	= 0;

		if(currentCell==NULL)
			throw Exception("Pipeline::process - No BufferCell was assigned.", __FILE__, __LINE__, Exception::CoreException);

		if(!GLEW_VERSION_3_3 && perfsMonitoring)
		{
			totalTiming = clock();
			timing = clock();
		}

		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "Pipeline::process - Processing : " << getFullName() << std::endl;
		#endif

		for(unsigned int k=0; k<actionsList.size(); k++)
		{
			ActionHub* 	action 	= &actionsList[k];
			Filter* 	f 	= filtersList[ action->filterIdx ];
			HdlFBO* 	t 	= currentCell->buffersList[ action->bufferIdx ];

			#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
				std::cout << "    applying filter : " << f->getFullName() << "..." << std::endl;
			#endif

			for(int l=0; l<f->getNumInputPort(); l++)
			{
				int bufferID 	= action->inputBufferIdx[l];
				int portID 	= action->inputArgumentIdx[l];

				#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
					std::cout << "        conecting buffer " << bufferID << " on port " << portID << std::endl;
				#endif

				if(bufferID==THIS_PIPELINE)
					f->setInputForNextRendering(l, inputsList[portID]);
				else
					f->setInputForNextRendering(l, (*currentCell->buffersList[bufferID])[portID]);
			}

			#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
				std::cout << "        Processing using buffer " << action->bufferIdx << "..." << std::endl;
			#endif

			if(perfsMonitoring)
			{
				if(GLEW_VERSION_3_3)
					glBeginQuery(GL_TIME_ELAPSED, queryObject);
				else
					timing = clock();

				//glFlush();
			}

			if(firstRun)
			{
				try
				{
					f->process(*t);
				}
				catch(Exception& e)
				{
					firstRun	= false;
					broken 		= true;
					Exception m("Pipeline::process - Exception caught in pipeline " + getFullName() + ", during processing : ", __FILE__, __LINE__, Exception::CoreException);
					m << e;
					throw m;
				}
			}
			else
				f->process(*t);

			if(perfsMonitoring)
			{
				//glFlush();

				if(GLEW_VERSION_3_3)
				{
					GLuint64 querytime;
					glEndQuery(GL_TIME_ELAPSED);
					glGetQueryObjectui64v(queryObject, GL_QUERY_RESULT, &querytime);
					perfs[k] = static_cast<double>(querytime)/1e6;
				}
				else
				{
					timing = clock() - timing;
					perfs[k] = static_cast<double>(timing)/static_cast<double>(CLOCKS_PER_SEC)*1000.0f;
				}
			}

			#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
				std::cout << "        Done." << std::endl;
			#endif
		}

		if(perfsMonitoring)
		{
			if(GLEW_VERSION_3_3)
			{
				totalPerf = 0;
				for(unsigned int i=0; i<perfs.size(); i++)
					totalPerf += perfs[i];
			}
			else
			{
				totalTiming = clock() - totalTiming;
				totalPerf   = static_cast<double>(totalTiming)/static_cast<double>(CLOCKS_PER_SEC)*1000.0f;
			}
		}

		#ifdef __GLIPLIB_DEVELOPMENT_VERBOSE__
			std::cout << "Pipeline::process - Done for pipeline : " << getFullName() << std::endl;
		#endif

		firstRun = false;
	}

	/**
	\fn Pipeline& Pipeline::operator<<(HdlTexture& texture)
	\brief Add a data as input to the pipeline. The user must maintain the texture in memory while this Pipeline hasn't received a Pipeline::Process or Pipeline::Reset signal.
	\param texture The data to use.
	\return This pipeline or raise an exception if any errors occur.
	**/
	Pipeline& Pipeline::operator<<(HdlTexture& texture)
	{
		if(static_cast<int>(inputsList.size())>=getNumInputPort())
			throw Exception("Pipeline::operator<<(HdlTexture&) - Too much arguments given to Pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);

		inputsList.push_back(&texture);

		return *this;
	}

	/**
	\fn Pipeline& Pipeline::operator<<(Pipeline& pipeline)
	\brief Add all the output of variable pipeline as input of this pipeline. The user must maintain the texture(s) in memory while this Pipeline hasn't received a Pipeline::Process or Pipeline::Reset signal.
	\param pipeline The pipeline outputs to append.
	\return This pipeline or raise an exception if any errors occur.
	**/
	Pipeline& Pipeline::operator<<(Pipeline& pipeline)
	{
		for(int i=0; i<pipeline.getNumOutputPort(); i++)
		{
			if(static_cast<int>(inputsList.size())>=getNumInputPort())
				throw Exception("Pipeline::operator<<(Pipeline&) - Too much arguments given to Pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);

			inputsList.push_back(&pipeline.out(i));
		}

		return *this;
	}

	/**
	\fn Pipeline& Pipeline::operator<<(ActionType a)
	\brief Apply operation on previously input data.
	\param a The ActionType (Process or Reset arguments).
	\return This pipeline or raise an exception if any errors occur.
	**/
	Pipeline& Pipeline::operator<<(ActionType a)
	{
		// Check the number of arguments given :
		if(static_cast<int>(inputsList.size())!=getNumInputPort() && a!=Reset)
			throw Exception("Pipeline::operator<<(ActionType) - Too few arguments given to Pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);

		switch(a)
		{
			case Process:
				process();
			case Reset:            // After Process do Reset of the Input
				cleanInput();
				break;
			default:
				throw Exception("Pipeline::operator<<(ActionType) - Unknown action for Pipeline " + getFullName() + ".", __FILE__, __LINE__, Exception::CoreException);
		}

		return *this;
	}

	/**
	\fn HdlTexture& Pipeline::out(int i, int cellID)
	\brief Return the output of the pipeline.
	\param i The ID of the output port.
	\param cellID Choose the target cell (default : current in-use cell).
	\return A reference to the corresponding output texture or raise an exception if any errors occur.
	**/
	HdlTexture& Pipeline::out(int i, int cellID)
	{
		checkOutputPort(i);

		if(cellID==0)
		{
			if(currentCell==NULL)
				throw Exception("Pipeline::out - Unable to get the output " + getOutputPortName(i) + " as no cell is currently in use.", __FILE__, __LINE__, Exception::CoreException);
			else
				return (*(*currentCell->buffersList[ outputsList[i].bufferIdx ])[ outputsList[i].outputIdx ]);
		}
		else
		{
			std::map<int, BuffersCell*>::iterator it = cells.find(cellID);

			if(it==cells.end())
				throw Exception("Pipeline::out - The cell ID " + toString(it->first) + " does not exist.", __FILE__, __LINE__, Exception::CoreException);
			else
				return (*(*it->second->buffersList[ outputsList[i].bufferIdx ])[ outputsList[i].outputIdx ]);
		}
	}

	/**
	\fn HdlTexture& Pipeline::out(const std::string& portName, int cellID)
	\brief Return the output of the pipeline.
	\param portName The name of the output port.
	\param cellID Choose the target cell (default : current in-use cell).
	\return A reference to the corresponding output texture or raise an exception if any errors occur.
	**/
	HdlTexture& Pipeline::out(const std::string& portName, int cellID)
	{
		int index = getInputPortID(portName);
		return out(index, cellID);
	}

	/**
	\fn Filter& Pipeline::operator[](int filterID)
	\brief Access to the filter of described index.
	\param filterID The index of the filter, obtained with AbstractPipelineLayout::getElementID.
	\return The index or raise an exception if any errors occur.
	**/
	Filter& Pipeline::operator[](int filterID)
	{
		try
		{
			return *filtersList[ filtersGlobalIDsList[filterID] ];
		}
		catch(Exception& e)
		{
			Exception m("Pipeline::operator[int] - Error while processing request on filter ID : " + toString(filterID) + ".", __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}
		catch(std::exception& e)
		{
			Exception m("Pipeline::operator[int] - Error while processing request on filtr ID : " + toString(filterID) + ".", __FILE__, __LINE__, Exception::CoreException);
			m << e;
			throw m;
		}
	}

	/**
	\fn bool Pipeline::wentThroughFirstRun(void) const
	\brief Check if the pipeline was already computed, at least once.
	\return True if the pipeline was already applied.
	**/
	bool Pipeline::wentThroughFirstRun(void) const
	{
		return !firstRun;
	}

	/**
	\fn bool Pipeline::isBroken(void) const
	\brief Check if the pipeline is broken (its initialization failed, or an error occured during its first run).
	\return True if the pipeline is broken and should not be used.
	**/
	bool Pipeline::isBroken(void) const
	{
		return broken;
	}

	/**
	\fn int Pipeline::createBuffersCell(void)
	\brief Create a new buffers cell for this pipeline.
	\return The ID of the new cell (larger or equal to 1).
	**/
	int Pipeline::createBuffersCell(void)
	{
		static int	staticCellID 	= 1;
		const int 	cellID 		= staticCellID;

		cells[cellID] = new BuffersCell(bufferFormats);

		staticCellID++;

		return cellID;
	}

	/**
	\fn int Pipeline::getNumBuffersCells(void) const
	\brief Get the number of buffers cells.
	\return The number of cells.
	**/
	int Pipeline::getNumBuffersCells(void) const
	{
		return cells.size();
	}

	/**
	\fn bool Pipeline::isBuffersCellValid(int cellID) const
	\brief Test if a cell ID is valid.
	\param cellID The targeted ID.
	\return True if the cell ID corresponds to an existing buffers cell.
	**/
	bool Pipeline::isBuffersCellValid(int cellID) const
	{
		return (cells.find(cellID) != cells.end());
	}

	/**
	\fn int Pipeline::getCurrentCellID(void) const
	\brief Get the ID of the buffers cell currently in use for rendering.
	\return The cell ID of the current buffers cell.
	**/
	int Pipeline::getCurrentCellID(void) const
	{
		if(currentCell==NULL)
			return 0;
		else
		{
			for(std::map<int, BuffersCell*>::const_iterator it=cells.begin(); it!=cells.end(); it++)
			{
				if(it->second==currentCell)
					return it->first;
			}

			throw Exception("Pipeline::getCurrentCellID - The current cell is not listed (internal error).", __FILE__, __LINE__, Exception::CoreException);
		}
	}

	/**
	\fn std::vector<int> Pipeline::getCellIDs(void) const
	\brief Get all the IDs of the available cells.
	\return A list of all the valid IDs.
	**/
	std::vector<int> Pipeline::getCellIDs(void) const
	{
		std::vector<int> results;

		for(std::map<int, BuffersCell*>::const_iterator it=cells.begin(); it!=cells.end(); it++)
			results.push_back( it->first );

		return results;
	}

	/**
	\fn void Pipeline::changeTargetBuffersCell(int cellID)
	\brief Change the rendering target to another buffers cell.
	\param cellID The targeted ID.
	**/
	void Pipeline::changeTargetBuffersCell(int cellID)
	{
		std::map<int, BuffersCell*>::iterator it = cells.find(cellID);

		if(it==cells.end())
			throw Exception("Pipeline::changeTargetBufferCell - The cell ID " + toString(it->first) + " does not exist.", __FILE__, __LINE__, Exception::CoreException);
		else
			currentCell = it->second;
	}

	/**
	\fn void Pipeline::removeBuffersCell(int cellID)
	\brief Remove a buffers cell. If the cell is in use, it is necessary that the user change the rendering target afterward.
	\param cellID The targeted ID.
	**/
	void Pipeline::removeBuffersCell(int cellID)
	{
		std::map<int, BuffersCell*>::iterator it = cells.find(cellID);

		if(it==cells.end())
			throw Exception("Pipeline::removeBufferCell - The cell ID " + toString(it->first) + " does not exist.", __FILE__, __LINE__, Exception::CoreException);
		else
		{
			if(currentCell==it->second)
				currentCell = NULL;
			
			delete it->second;
			it->second = NULL;

			cells.erase(it);
		}
	}

	/**
	\fn void Pipeline::enablePerfsMonitoring(void)
	\brief Enable performances monitoring.

	Any following process on this instance will record time to perform the pipeline (time per element and total time). Calling this function twice will not reset previous results.
	**/
	void Pipeline::enablePerfsMonitoring(void)
	{
		if(!perfsMonitoring)
		{
			perfsMonitoring = true;
			perfs.assign(filtersList.size(),0.0f);
			totalPerf = 0.0f;

			if(GLEW_VERSION_3_3)
			{
				if(queryObject==0)
					glGenQueries(1, &queryObject);
			}
		}
	}

	/**
	\fn void Pipeline::disablePerfsMonitoring(void)
	\brief Disable performances monitoring.

	Stops a monitoring session on this instance. Do nothing if no session was started.
	**/
	void Pipeline::disablePerfsMonitoring(void)
	{
		if(perfsMonitoring)
		{
			perfsMonitoring = false;
			perfs.clear();
			totalPerf = 0.0;
		}
	}

	/**
	\fn double Pipeline::getTiming(int filterID)
	\brief Get last result of performance monitoring IF it is still enabled.
	\param filterID The ID of the filter.
	\return Time in milliseconds needed to apply the filter (not counting binding operation).
	**/
	double Pipeline::getTiming(int filterID)
	{
		if(perfsMonitoring)
			return perfs[ filtersGlobalIDsList[filterID] ];
		else
			throw Exception("Pipeline::getTiming - Monitoring is disabled.", __FILE__, __LINE__, Exception::CoreException);
	}

	/**
	\fn double Pipeline::getTiming(int action, std::string& filterName)
	\brief Get last result of performance monitoring IF it is still enabled.
	\param action The ID of the filter.
	\param filterName A reference string that will contain the name of the filter indexed by action at the end of the function.
	\return Time in milliseconds needed to apply the filter (not counting binding operation).
	**/
	double Pipeline::getTiming(int action, std::string& filterName)
	{
		if(perfsMonitoring)
		{
			if(action<0 || action>=static_cast<int>(actionsList.size()))
				throw Exception("Pipeline::getTiming - Action index is outside of range.", __FILE__, __LINE__, Exception::CoreException);
			else
			{
				filterName = filtersList[ actionsList[action].filterIdx ]->getFullName();
				return perfs[action];
			}
		}
		else
			throw Exception("Pipeline::getTiming - Monitoring is disabled.", __FILE__, __LINE__, Exception::CoreException);
	}


	/**
	\fn double Pipeline::getTotalTiming(void)
	\brief Get total time elapsed for last run.
	\return Time in milliseconds needed to apply the whole pipeline (counting everything and flushing after each filter).
	**/
	double Pipeline::getTotalTiming(void)
	{
		if(perfsMonitoring)
			return totalPerf;
		else
			throw Exception("Pipeline::getTotalTiming - Monitoring is disabled.", __FILE__, __LINE__, Exception::CoreException);
	}

