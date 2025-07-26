#ifndef CONFIG_FILE
#warning "CONFIG_FILE not defined"
extern ollist_t oll;
int olLoadConfig(char *filename, ollist_t *oll, size_t size,char *errmsg);
int olWriteConfig(char *filename, ollist_t *oll, size_t size,char *errmsg);
int oWriteConfig(char *filename, O_DATA *curr_o_array,size_t size,char *errmsg);
int oWriteConfigXML(char *filename, O_DATA *curr_o_array,size_t size,char *errmsg);
#else
#warning "CONFIG_FILE defined"
#endif
int oLoadConfig(char *filename, O_DATA *curr_o_array,size_t size,char *errmsg);
int oWriteConfig(char *filename, O_DATA *curr_o_array,size_t size,char *errmsg);
int oWriteConfigXML(char *filename, O_DATA *curr_o_array,size_t size,char *errmsg);
int GetFileFormat(char *filename);
int getFileCreationTime(char *path,char *str);
int WriteParams(char *filename, PARAM_STRUCT *ps, char *password, char *errmsg);
char *LoadParams(char *filename, PARAM_STRUCT *ps, char *password, char *errmsg);
int LoadSpecialInputFunctions(IP *ip, int no_current_ips);