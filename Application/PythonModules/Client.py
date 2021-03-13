import threading
import socket
import cPickle
import time

from Globals import *


class Client(threading.Thread):
	"""extension of thread object to start up and run our
	socket client"""

	def __init__(self, data):
		threading.Thread.__init__(self)
		self.data = data

	def run(self):
		s_socket = ClientSocket()
		# acquire and send data
		s_socket.send(self.data)
		# recieve acknoledgement / server messages
		data = s_socket.recv()
		print data


class ClientSocket(object):

	"""socket server that sends and recieves pickled python objects"""
	def __init__(self):
		self.sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
		self.sock.connect(('localhost', 10890))
		self.SMSG_ACKNOWLEDGE = '!RECV!'
		self.END = '[!END!]'

	def recv(self):
		XSI.LogMessage("ELIXSIClientSocket::RecV")
		"""recv data on sock, using an end identifier to acknowledge
		when complete message is done"""

		total_data=[]
		data=''
		recv_active=True

		while True:
			try:
				data=self.sock.recv(8192)
				if self.END in data:
					total_data.append(data[:data.find(self.END)])
					break
				total_data.append(data)
				if len(total_data)>1:
					# check if end_of_data was split
					last_pair = total_data[-2]+total_data[-1]
					if self.END in last_pair:
						total_data[-2] = last_pair[:last_pair.find(self.END)]
						total_data.pop()
						break
			except socket.error, data:
				if data[0] == 10035:
					# non blocking socket, hold for messages
					time.sleep(1)
				else:
					print('ELIXSIClient : error on recieve')
					break

		msg=cPickle.loads(''.join(total_data))
		return msg

	def send(self, data):
		XSI.LogMessage(data)
		try:
			p_data = cPickle.dumps(data)
			XSI.LogMessage(p_data)
			print("sending!")
			self.sock.sendall(p_data+self.END)
			print('ELIXSIClient : sent',p_data+self.END)
		except socket.error, data:
			print('ELIXSIClient : error on send')


a = Client(['test data',{'benmalartre':'awesome', 'bubba':'awesomeo'}])
a.start()

