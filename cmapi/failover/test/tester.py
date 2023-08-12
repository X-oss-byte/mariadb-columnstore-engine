from .. import config
import time
from socket import *
import struct
import sys

_config = config.Config("failover/test/config-test.xml")
print(f"got desired_nodes = {_config.getDesiredNodes()}")
print(f"got active_nodes = {_config.getActiveNodes()}")
print(f"got inacive_nodes = {_config.getInactiveNodes()}")
print(f"got all nodes = {_config.getAllNodes()}")
print(f"got primarynode = {_config.getPrimaryNode()}")
print()

from ..heartbeater import HeartBeater
from ..heartbeat_history import HBHistory

hbh = HBHistory()
hb = HeartBeater(_config, hbh)
hb.start()
sock = socket(type = SOCK_DGRAM)
sock.bind(('localhost', 12345))

# Updated heartbeater to send the reply to its own port, rather than to
# the port of the sending socket.  Need to update this.
#msg = struct.pack("4sH", hb.areYouThereMsg, 1234)
#sock.sendto(msg, ('localhost', hb.port))
#print("sent the are-you-there msg")
#(data, remote) = sock.recvfrom(6)
#(data, seq) = struct.unpack("4sH", data)
#if data == hb.yesIAmMsg:
#    print("got the yes-i-am msg, seq = {}".format(seq))
#else:
#    print("got something other than the yes-i-am-msg")

hb.stop()

#from heartbeat_history import HBHistory

#hbh = HBHistory()
hbh.setCurrentTick(0)
hbh.gotHeartbeat("node1", 0)
hbh.setCurrentTick(1)
hbh.gotHeartbeat("node2", 1)
hbh.setCurrentTick(2)
hbh.setCurrentTick(10)
hbh.gotHeartbeat("node1", 9)
hbh.gotHeartbeat("node1", 2)
pongs = hbh.getNodeHistory("node1", 20)
print(f"Got pongs: {pongs}")

print('''
 This is currently a 'manual' test, meaning the user should watch for the expected output
 In this case, because NM's identity checker will return 'node1', and that does not match
 node[2-4], those nodes will appear to NodeMonitor to be offline.  Our starting condition
 is that nodes 1-3 are active, and node4 is inactive.  After 15s, nodes 2 & 3
 should be deactivated, a new primary node will be chosen, and and our AgentBase will start 
 printing these events.
''')
def testNodeMonitor1(nm):
    nm.start()
    print("Waiting for 20 secs, watch for output from AgentBase")
    time.sleep(20)
    nm.stop()
    time.sleep(1)
    print("NodeMonitor was stopped, did it produce the right output?")

from ..node_monitor import NodeMonitor
nm = NodeMonitor(config = _config, samplingInterval = 10)
# check whether node[1-4] are in the /etc/hosts file as localhost
addr1 = gethostbyname("node1")
addr2 = gethostbyname("node2")
addr3 = gethostbyname("node3")
addr4 = gethostbyname("node4")
if addr1 == '127.0.0.1' and addr2 == '127.0.0.1' and addr3 == '127.0.0.1' and addr4 == '127.0.0.1':
    testNodeMonitor1(nm)
else:
    print("Skipping testNodeMonitor1().  node[1-4] needs to be defined as 127.0.0.1 in /etc/hosts")



print("tester is finished")

