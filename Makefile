
default: pingTool

pingTool:
	g++ PingTool.cpp -o pingTool
	echo "Format is ./pingTool {ipv4 address}"
