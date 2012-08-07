#ifndef __CRWOBJECTMODEL_H__
#define	__CRWOBJECTMODEL_H__

#include "Crown.h"

using namespace Crown;

class CRWObjectModel;

class CRWDescriptor: public WithProperties
{
public:
	CRWDescriptor(CRWObjectModel* crwObjectModel, CRWDescriptor* parent, const Descriptor& descriptor, uint offset);
	virtual ~CRWDescriptor();

	const List<CRWDescriptor*>& GetChildren();

	Str GetFullName();
	CRWObjectModel* GetCRWObjectModel();
	inline ushort GetType()
	 { return mType; }

	Str ToStr() const;

	virtual void OnGetProperty(const Str& name);

	virtual Generic GetPropertyValue(const Str& name) const;


private:
	CRWObjectModel* mCRWObjectModel;
	CRWDescriptor* mParent;
	List<CRWDescriptor*>* mChildren;
	Shared<IList<Generic> > mChildrenGenericWrapper;

	//Descriptor Properties
	uint mID;
	ushort mType;
	Str mName;
	uint mContentOffset;
	uint mContentSize;
	//TODO: Use uchar
	ushort mFlags;
	uint mDescriptorOffset;

	friend class CRWObjectModel;
};

class CRWObjectModel: public IWithProperties
{
public:
	CRWObjectModel(CRWDecoder* decoder);
	virtual ~CRWObjectModel();

	inline const IList<CRWDescriptor*>* GetRoot()
	{
		if (mRoot.IsNull())
			mRoot = LoadChildren(NULL);
		return mRoot.GetPointer();
	}

	Str GetCRWLibraryPath();

	virtual Generic GetPropertyValue(const Str& name) const;
	virtual void SetPropertyValue(const Str& name, const Generic& value);

	virtual Str ToStr() const
	{
		return "CRWObjectModel";
	}

private:
	CRWDecoder* mDecoder;
	Shared<IList<CRWDescriptor*> > mRoot;

	List<CRWDescriptor*>* LoadChildren(CRWDescriptor* descriptor);

	friend class CRWDescriptor;
};

inline const List<CRWDescriptor*>& CRWDescriptor::GetChildren()
{
	if (!mChildren)
		mChildren = mCRWObjectModel->LoadChildren(this);
	return *mChildren;
}

#endif	/* __CRWOBJECTMODEL_H__ */

