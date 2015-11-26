import hazelcast
config = hazelcast.ClientConfig()
client = hazelcast.HazelcastClient(config)
map = client.getIntMap("ihsan")
key = 5
value = 7
map.put(key, value)
returnedValue = map.get(key)
print "Value for key %d is %d" %(key, returnedValue)
