namespace DL_List {
	struct Node {
		Node *next, *prev;
		void *data;
	};

	struct List {
		int size;
		struct Node *head;
		struct Node *tail;
	};

	List *create(void);
	void remove(List *list, Node *n);
	void add_front(List *list, Node *n);
	void add_after(List *list, Node *insert_point, Node *n);
	void add(List *list, Node *n);
	void dump(List *list);
} // end namepsace

