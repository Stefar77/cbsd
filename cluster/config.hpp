#ifndef CONFIG_HPP
#define CONFIG_HPP

#define CBSDROOT "/usr/jails"

#define	ControllerIP "127.0.0.1"
#define	ControllerPORT 1234
#define	ClusterCA "/etc/ssl/clusterca.crt"

#define	ControllerCRT "/etc/ssl/Controller.crt"
#define	ControllerKEY "/etc/ssl/Controller.key"
#define	ControllerPassword "geheim"

#define	NodeCRT "/etc/ssl/SuperBSD.crt"
#define	NodeKEY "/etc/ssl/SuperBSD.key"
//#define	NodeCRT "/etc/ssl/GUI.crt"
//#define	NodeKEY "/etc/ssl/GUI.key"
#define	NodePassword "geheim"

/* REDIS CONFIG */
#define RedisIP "127.0.0.1"
#define RedisPORT 6379
#define RedisPassword "password"
#define RedisDatabase 2
 // Redis options
#define RedisEQ "cbsd_events"
 // Fire events in redis for the following...
#define RedisEQ_Controller_State 
#define RedisEQ_Node_State
 // Does not work yet
#define RedisEQ_Jail_State

 // If defined it tries to get and set info on this hash for the node {PRE}{NODE_NAME}.item=value
#define RedisDB_Node_HashPre	"NODE:"




/* END OF REDIS CONFIG */


#define LoggingLevel DEBUG

#endif
