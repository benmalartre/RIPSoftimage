import sys
import threading
import time
import socket
import cPickle
import win32com

from Globals import *

null = None
false = 0
true = 1

def XSILoadPlugin( in_reg ):
	in_reg.Author = "jules_stevenson"
	in_reg.Name = "ELIXSIServer"
	in_reg.Major = 1
	in_reg.Minor = 2

	in_reg.RegisterCommand("ELIServerStop" )
	in_reg.RegisterCommand("ELIServerStart" )
	in_reg.RegisterTimerEvent("ELIServerProcess", 100)
	esp = Application.EventInfos("ELIServerProcess")
	esp.mute = True

	#RegistrationInsertionPoint - do not remove this line
	return true

def XSIUnloadPlugin( in_reg ):
	strPluginName = in_reg.Name
	xsi.LogMessage(str(strPluginName) + str(" has been unloaded."))
	return true

# ---------------------------
# register stuff for commands
# ---------------------------

def ELIServerStart_Init( in_ctxt ):
	oCmd = in_ctxt.Source
	oCmd.Description = "Starts the server"
	oCmd.ReturnValue = true
	return true

def ELIServerStop_Init( in_ctxt ):
	oCmd = in_ctxt.Source
	oCmd.Description = "Stops the server"
	oCmd.ReturnValue = true
	return true

def ELIServerStart_Execute(  ):
	# start the server
	xsi_server = XSI_server()
	xsi_server.start()
	# set the server to the plugin user data [persistance]
	plugin = Application.Plugins("ELIXSIServer")
	plugin.UserData = win32com.server.util.wrap(XSIUserStore(xsi_server))

	# set off the timer
	esp = Application.EventInfos("ELIServerProcess")
	esp.mute = False

def ELIServerStop_Execute(  ):
	plugin = Application.Plugins("ELIXSIServer")
	xsi_server = plugin.UserData
	xsi_server.stop_server()
	# turn off the timer
	esp = Application.EventInfos("ELIServerProcess")
	esp.mute = True

# Callback for the siOnTimerEvent event.
def ELIServerProcess_OnEvent( in_ctxt ):
	# check tasks
	plugin = Application.Plugins("ELIXSIServer")
	userdata = plugin.UserData

	while len(userdata) > 0:
		# get task and unpickle (done here not in recieve because the results may
		# not be hashable to a com variant, so we bypass the risk)
		
		task = cPickle.loads(str(userdata.pop_task(0)))
		
		##################################
		#
		# Process task code here
		#
		##################################
		
		xsi.LogMessage (task)		

# 	This event can be aborted by returning true or false if you don't want to abort.
	return false

# -------------------------------------------------------------------------
# Global communication variables
# -------------------------------------------------------------------------

class Comms(object):
	SMSG_ACKNOWLEDGE = '!RECV!'
	CMSG_SHUTDOWN = '!SHUT_DOWN!'
	END = '[!END!]'
	DOMAIN = 'localhost'
	PORT = 10890

# -----------------------------------------------------------------------
# main xsi server class
# -----------------------------------------------------------------------

class XSI_server(threading.Thread, Comms):

	"""BASE SERVER CLASS"""
	
	# Declare list of exported functions:
	_public_methods_ = ['run', 'stop']
	# Declare list of exported attributes
	_public_attrs_ = ['name']
	# Declare list of exported read-only attributes:
	_readonly_attrs_ = []

	def __init__(self):
		threading.Thread.__init__(self)
		self.serve = True
		self.sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)

	def run(self):
		xsi.LogMessage ("ELIXSIServer : Started")
		try:
			self.sock.bind((Comms.DOMAIN, Comms.PORT))
			self.sock.listen(5)

			while self.serve:
				commsock, address=self.sock.accept()
				connection=ServerSocket(commsock,address)
				connection.start()
		except socket.error, data:
			xsi.LogMessage ('ELIXSIServer : ERROR: %s, %s' (data[0], data[1]))

	def stop(self):
		self.serve = False
		# send a client to the server (this) to break the wait on connection 
		# and enable a proper shutdown
		end_connector = ClientSocket('localhost')
		end_connector.send(Comms.END)
		xsi.LogMessage ("ELIXSIServer : Stopped")

class ServerSocket(threading.Thread, Comms):
	
	"""connection thread. Something to do with threading means this does
	not report excpetions - it just silently disappears... Need to investigate
	this and tidy up the exception handling"""
	
	def __init__(self, sock, address):
		threading.Thread.__init__(self)
		self.name = 'XSI_SERVER_THREAD'
		self.sock = sock
		self.IP = address[0]
		self.sock.setblocking(False)

	def run(self):
		try:
			data = self.recv()
			xsi.LogMessage(data)
			# process data
			plugin = disp(Application.Plugins("KETTLE_xsi_server"))
			userdata = plugin.UserData
			userdata.append_task(data)
			
			self.send(Comms.SMSG_ACKNOWLEDGE)
		except error, data:
			xsi.LogMessage ("ELIXSIServer_SOCKET : ERROR : %s, %s" % (data[0], data[1]))
		

	def recv(self):
	
		"""recv data on sock, using an end identifier to acknowledge
		when complete message is done"""

		total_data=[]
		data=''

		while True:
			try:
				data=self.sock.recv(8192)
				if Comms.END in data:
					total_data.append(data[:data.find(Comms.END)])
					break
				total_data.append(data)
				if len(total_data)>1:
					#check if end_of_data was split
					last_pair=total_data[-2]+total_data[-1]
					if Comms.END in last_pair:
						total_data[-2]=last_pair[:last_pair.find(Comms.END)]
						total_data.pop()
						break
			except socket.error, data:
				if data[0] == 10035:
					# non blocking socket, hold for messages
					time.sleep(0.1)
				else:
					xsi.LogMessage('ELIXSIServer_SOCKET (ERROR): %s, %s' % (data[0], data[1]))

		return ''.join(total_data)

	def send(self, data):
		try:
			p_data = cPickle.dumps(data)
			self.sock.sendall(p_data+Comms.END)
		except socket.error, data:
			xsi.LogMessage('ELIXSIServer_SOCKET (ERROR): %s, %s' % (data[0], data[1]))


# ----------------------------------------------------------------------
# clientsocket used to shut down the server
# ----------------------------------------------------------------------

class ClientSocket(Comms):

    """socket server that sends pickled python objects"""
	
    def __init__(self, ip):
	self.sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	self.sock.connect((ip, 10890))

    def send(self, data):
	try:
	    p_data = cPickle.dumps(data)
	    self.sock.sendall(p_data+Comms.END)
	except socket.error, data:
		xsi.LogMessage('ELIXSIServer_SHUTDOWN_SOCKET (ERROR): %s, %s' % (data[0], data[1]))
			
# ------------------------------------------------------------------------
# xsi compatible threadsafe objects (maybe)
# ------------------------------------------------------------------------

class XSIUserStore(object):
	"""class to pass around data, specifically the task list. Has 
	built in thread lock in case of the rare case of multiple connections
	to the server at once"""
	
	_public_methods_ = ['stop_server', 'append_task', 'pop_task', 'count']
	_public_attrs_ = ['tasks', 'lock', 'server']
	
	def __init__(self, server):
		self.tasks = []
		self.lock = threading.Lock()
		self.server = server
		
	def stop_server(self):
		self.server.stop()
		
	def append_task(self, task):
		self.lock.acquire()
		self.tasks.append(task)
		self.lock.release()
		
	def pop_task(self, index):
		self.lock.acquire()
		item = self.tasks.pop(index)
		self.lock.release()
		return item
	
	def count(self):
		self.lock.acquire()
		item = len(self.tasks)
		self.lock.release()
		return item
