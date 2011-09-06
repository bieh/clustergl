################################################################################ 
# Configuration
################################################################################

#Location of the ClusterGL install to run
CGL_LOCATION = "/home/paul/Projects/clustergl2"

#Default configuration file
CGL_CONFIG_FILE = CGL_LOCATION + "/runtime/single.conf"

#Hostnames or IPs of rendering machines that we will ssh into
#CGL_RENDERERS = ["dn1", "dn2", "dn3", "dn4", "dn5"]
CGL_RENDERERS = ["localhost"]

CGL_PID_FILE = "/tmp/cglcapture.pid"
