#include "CRWObjectModel.h"

CRWDescriptor::CRWDescriptor(CRWObjectModel* crwObjectModel, CRWDescriptor* parent, const Crown::Descriptor& descriptor, Crown::uint offset):
	mCRWObjectModel(crwObjectModel), mParent(parent), mChildren(NULL), mDescriptorOffset(offset)
{
	AddProperty(new UIntProperty("ID", &mID));
	AddProperty(new UShortProperty("Type", &mType));
	AddProperty(new StrProperty("Name", &mName));
	AddProperty(new UIntProperty("ContentOffset", &mContentOffset));
	AddProperty(new UIntProperty("ContentSize", &mContentSize));
	AddProperty(new UShortProperty("Flags", &mFlags));
	AddProperty(new UIntProperty("DescriptorOffset", &mDescriptorOffset));

	AddProperty(new GenericListProperty("Children", &mChildrenGenericWrapper));

	mID = descriptor.ID;
	mType = descriptor.type;
	mName = descriptor.name;
	mContentOffset = descriptor.contentOffset;
	mContentSize = descriptor.contentSize;
	mDescriptorOffset = offset;
}

CRWDescriptor::~CRWDescriptor()
{
	delete mChildren;
}

Crown::Str CRWDescriptor::ToStr() const
{
	return "CRWDescriptor\"" + mName + "\"";
}

void CRWDescriptor::OnGetProperty(const Crown::Str& name)
{
	if (name == "Children")
	{
		if (mChildren == NULL)
		{
			GetChildren();
			mChildrenGenericWrapper = new Crown::ListGenericWrapper<CRWDescriptor*>(mChildren);
		}
	}
}

Generic CRWDescriptor::GetPropertyValue(const Str& name) const
{
	return WithProperties::GetPropertyValue(name);
}

Crown::Str CRWDescriptor::GetFullName()
{
	Crown::Str name;
	name = mName;
	
	if (!mParent)
		return "/" + name;
	else
		return mParent->GetFullName() + "/" + name;
}

CRWObjectModel* CRWDescriptor::GetCRWObjectModel()
{
	return mCRWObjectModel;
}

Crown::List<CRWDescriptor*>* CRWObjectModel::LoadChildren(CRWDescriptor* crwDescriptor)
{
	Crown::Str path = "/";
	if (crwDescriptor)
	{
		path = crwDescriptor->GetFullName();
	}

	Crown::List<CRWDescriptor*>* list = new Crown::List<CRWDescriptor*>();
	//If seek fails, the path is not a directory and therefore there's no children
	if (!mDecoder->Seek(path))
		return list;

	while (mDecoder->NextDescriptor())
	{
		const Crown::Descriptor& descriptor = mDecoder->GetDescriptor();
		Crown::uint offset = mDecoder->GetDescriptorOffset();
		list->Append(new CRWDescriptor(this, crwDescriptor, descriptor, offset));
	}

	return list;
}

CRWObjectModel::CRWObjectModel(Crown::CRWDecoder* decoder):
	mDecoder(decoder), mRoot(NULL)
{
	GetRoot();
}

CRWObjectModel::~CRWObjectModel()
{
}

Str CRWObjectModel::GetCRWLibraryPath()
{
	return mDecoder->GetLibraryPath();
}

Generic CRWObjectModel::GetPropertyValue(const Str& name) const
{
	if (name == "Root")
	{
		//This does not generate leaks because Generic stores the pointer in a Shared<>
		return new ListGenericWrapper<CRWDescriptor*>(mRoot.GetPointer());
	}
	return Generic();
}

void CRWObjectModel::SetPropertyValue(const Str& name, const Generic& value)
{

}


