class Message;

// I havn't add assignment operator and copy constructor
// destructor needed too.
class Folder {
public:
	void addMsg(Message *);
	void remMsg(Message *);
private:
	set<Message *> messages;
};