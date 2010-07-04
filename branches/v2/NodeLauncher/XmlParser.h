/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v2.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* All rights reserved.                                                        *
*                                                                             *
* Redistribution and use in source and binary forms, with or without modifi-  *
* cation, are permitted provided that the following conditions are met:       *
*                                                                             *
*    * Redistributions of source code must retain the above copyright notice  *
*      this list of conditions and the following disclaimer.                  *
*                                                                             *
*    * Redistributions in binary form must reproduce the above copyright      *
*      notice, this list of conditions and the following disclaimer in the    *
*      documentation and/or other materials provided with the distribution.   *
*                                                                             *
*    * Neither the name of RPISEC nor the names of its contributors may be    *
*      used to endorse or promote products derived from this software without *
*      specific prior written permission.                                     *
*                                                                             *
* THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY EXPRESS OR IMPLIED      *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN     *
* NO EVENT SHALL RPISEC BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR      *
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        *
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          *
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                *
*                                                                             *
*******************************************************************************
*                                                                             *
* XmlParser.h - XML parsing library                                           *
*                                                                             *
******************************************************************************/


#ifndef xmlparser_h
#define xmlparser_h

//Disable stupid VC++ warnings - we want to be portable, not use MS's nonstandard functions.
//I can check for buffer overflows myself on G++ - why not here too?
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <string>
#include <vector>

class XmlTagAttribute
{
public:
	XmlTagAttribute();
	XmlTagAttribute(std::string n,std::string v)
		: m_name(n)
		, m_value(v)
	{}

	std::string m_name;
	std::string m_value;
};

class XmlNode
{
public:
	XmlNode(
		std::string& xml,
		unsigned int& i,
		unsigned int& line);
	virtual ~XmlNode();

	enum nodetype
	{
		NODETYPE_TAG,
		NODETYPE_TEXT,
		NODETYPE_COMMENT
	};

	//Returns false if EOF encountered during load
	bool IsValid() const
	{ return m_bValid; }

	//Returns our tag type
	const std::string& GetType() const
	{ return m_type; }

	//Returns our node type
	nodetype GetNodeType() const
	{ return m_nodetype; } 

	//Returns our body
	const std::string& GetBody() const
	{ return m_body; }

	//Returns our number of child nodes
	unsigned int GetChildCount() const
	{ return m_nodes.size(); }

	//Returns our Nth child node
	const XmlNode* GetChildNode(unsigned int n) const
	{ return m_nodes[n];  }

	//Returns our number of attributes
	unsigned int GetAttributeCount() const
	{ return m_attribs.size(); }

	//Returns our Nth attribute
	const XmlTagAttribute& GetAttribute(unsigned int n) const
	{ return m_attribs[n]; }

	//Returns the name of our Nth attribute
	const std::string& GetAttributeName(unsigned int n) const
	{ return m_attribs[n].m_name; }

	//Returns the value of our Nth attribute
	const std::string& GetAttributeValue(unsigned int n) const
	{ return m_attribs[n].m_value; }

protected:
	std::vector<XmlTagAttribute> m_attribs;	//List of attributes

	nodetype m_nodetype;			//Type of node
	
	std::string m_body;				//Body of text nodes and comments

	std::string m_type;				//Type if we're a tag, otherwise empty

	bool m_bSingleTag;				//True if we're a self-closing tag like <br/>
	bool m_bValid;					//True if we're well formed and not EOF

	std::vector<XmlNode*> m_nodes;		//Our nodes

	void EatSpaces(					//Skip spaces, updating line numbers as needed
		std::string& xml,
		unsigned int& i,
		unsigned int& line);
};

class XmlParser
{
public:
	XmlParser(const char* fname);			//no default constructor - XmlParser objects must be initialized at creation
	XmlParser(std::string& xml);

private:
	XmlParser(const XmlParser& rhs)
	{ throw std::string("XmlParser objects cannot be copied"); }
	XmlParser& operator=(const XmlParser& rhs)
	{ throw std::string("XmlParser objects cannot be copied"); }

public:
	virtual ~XmlParser();

	XmlNode* GetRoot()
	{ return m_root; }

	XmlNode* GetDoctype()
	{ return m_doctype; }

	XmlNode* GetXmlTag()
	{ return m_xmltag; }

protected:
	XmlNode* m_root;
	XmlNode* m_doctype;
	XmlNode* m_xmltag;

	void Load(std::string& xml);	//The core loading code
};

#endif
