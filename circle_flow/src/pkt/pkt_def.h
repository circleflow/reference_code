
#ifndef PKT_DEF_H_
#define PKT_DEF_H_

#define PKT_DEF_IMPL

#define BLOCK_START(name) FIELD_BLOCK fb(name);

#define FIXED_SIZE(name,size,data)             fb.append(FIELD_FIXED_SIZE(name,size)),fb.field(name)=data;
#define RESIZABLE(name,size)                   fb.append(FIELD_RESIZABLE(name,size));
#define OPTION(name,size)                      fb.append(FIELD_OPTION(name,size));
#define CACULATOR(name,size,fields,filter,cac) fb.append(FIELD_CACULATOR(name,size,fields,filter,cac));

#define BLOCK_END


#endif
