#ifndef CONFIG_HPP
#define CONFIG_HPP

#define	ControllerIP "127.0.0.1"
#define	ControllerPORT 1234
#define	ClusterCA "/etc/ssl/clusterca.crt"

#define	ControllerCRT "/etc/ssl/Controller.crt"
#define	ControllerKEY "/etc/ssl/Controller.key"
#define	ControllerPassword "geheim"

#define	NodeCRT "/etc/ssl/GUI.crt"
#define	NodeKEY "/etc/ssl/GUI.key"
#define	NodePassword "geheim"

#define RedisIP "127.0.0.1"
#define RedisPORT 6379
#define RedisPassword "password"
#define RedisDatabase 2


#define LoggingLevel DEBUG


#endif
