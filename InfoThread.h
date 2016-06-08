typedef struct InfoThread{
	char isServer;
	union{
		struct{
			int logFile;
			int sock;
			int thread_id;
			char isFree;
		}InfoThreadC;

		struct{
			int logFile;
			int sock;
			int thread_id;
			char isFree;
			int connexionPort;
		}InfoThreadS;
	};
}InfoThread;
