###### A plugin that will not allow a raksamp to enter

###### It works by going outside the array: https://github.com/YashasSamaga/RakSAMP/blob/master/client/src/netgame.cpp#L64
###### The developer did not take into account that an ID value greater than 1004 may arrive
###### The usual SA:MP client does not manipulate the nickname when processing ID_CONNECTION_REQUEST_ACCEPTED. Due to this, we can just send 65535, and the raksamp will crash
