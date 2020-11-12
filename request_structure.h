#ifndef REQUEST_STRUCTURE_H
#define REQUEST_STRUCTURE_H
#define BUFSIZE 512


class RequestData {
  public:
    // Contents of request data? Time duration?? Counter?ty
    int test;
    char port [BUFSIZE];

    // Encode to a buffer of uint64_t.
    void encode(char *buf)
    {
        int *q = (int*)buf;    
        *q = test;       q++; 

        char *p = (char*)q;
        int i = 0;
        while (i < BUFSIZE)
        {
            *p = port[i];
            p++;
            i++;
        }
    }

    // Decode from a buffer of uint64_t.
    void decode(char* buf) {
        int *q = (int*)buf;    
        test = *q;       q++;    

        char *p = (char*)q;
        int i = 0;
        while (i < BUFSIZE)
        {
            port[i] = *p;
            p++;
            i++;
        }
    }
};


#endif
