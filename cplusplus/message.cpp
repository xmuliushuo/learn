#include "message.h"

void Message::put_Msg_in_Folders(const std::set<Folder *> &rhs)
{
	for(std::set<Folder *>::const_iterator beg = rhs.begin();
		beg != rhs.end(); ++beg)
		(*beg)->addMsg(this);
}

Message::Message(const Message &m):
	content(m.content), folders(m.folders)
{
	put_Msg_in_Folders(folders);
}

Message& Message::operator=(const Message &rhs)
{
	if (&rhs != this) {
		remove_Msg_from_Folders();
		contents = rhs.contents;
		folders = rhs.folders;
		put_Msg_in_Folders(rhs.folders);
	}
	return *this;
}

void Message::remove_Msg_from_Folders()
{
	for(std::set<Folder *>::const_iterator beg = folders.begin();
		beg != folders.end(); ++beg)
		(*beg)->remMsg(this);
}

Message::~Message()
{
	remove_Msg_from_Folders();
}

void Message::addFldr(Folder *folder)
{
	folders.insert(folder);
}

void Message::remFldr(Folder *folder)
{
	folders.erase(folder);
}

void Message::save(Folder& folder)
{
	addFldr(&folder);
	folder.addMsg(this);
}

void Message::remove(Folder& folder)
{
	remFldr(&folder);
	folder.remMsg(this);
}	