import java.net.*; // for Socket, ServerSocket, and InetAddress 
import java.io.*; // for IOException and Input/OutputStream 
import java.nio.ByteBuffer; 
public class ServerTCP { 
	private static final int BUFSIZE = 32; // Size of receive buffer 
	public static void main(String[] args) throws IOException {
 		if (args.length != 1) { // Test for correct # of args 
          	throw new IllegalArgumentException("Parameter(s): <Port>"); 
        }
          	
		int servPort = Integer.parseInt(args[0]); 

        // Create a server socket to accept client connection requests 
		ServerSocket servSock = new ServerSocket(servPort);
		int recvMsgSize; // Size of received message 
		byte[] byteBuffer = new byte[1024]; // Receive buffer 
		int req_id = 0;

		for (;;) { // Run forever, accepting and servicing connections 
			Socket clntSock = servSock.accept(); // Get client connection 

			System.out.println("Handling client at " + 
             	clntSock.getInetAddress().getHostAddress() 
             	+ " on port " + clntSock.getPort()); 

			InputStream in = clntSock.getInputStream();
			OutputStream out = clntSock.getOutputStream();
			BufferedInputStream bufIn = new BufferedInputStream(in);

			//Send Opcode request
			String req_opcode = "Please enter the Opcode: ";
			byteBuffer = req_opcode.getBytes("UTF-8");
			out.write(byteBuffer);

			//Receive Opcode
			int opcode = bufIn.read();
			System.out.println("Client Opcode: ");
			System.out.println(opcode);


			//Send num of operands request
			BufferedInputStream bufNumops = new BufferedInputStream(in);
			String req_numops = "Please enter the number of operands: ";
			byteBuffer = req_numops.getBytes("UTF-8");
			out.write(byteBuffer);

			//Receive num of operands
			int numops = bufNumops.read();
			System.out.println("Client number of operands: ");
			System.out.println(numops);


			//TML
			int tml = 0;
			if (numops == 2) {
				tml = 8;
			}
			else {
				tml = 6;
			}


			//Send first operand request
			BufferedInputStream bufop1 = new BufferedInputStream(in);
			String req_op1 = "Please enter the first operand: ";
			byteBuffer = req_op1.getBytes("UTF-8");
			out.write(byteBuffer);

			//Receive first operand
			int op1 = bufop1.read();
			System.out.println("Client first operand: ");
			System.out.println(op1);


			//Send second operand request, if necessary
			int op2 = 0;
			if (numops == 2) {
				BufferedInputStream bufop2 = new BufferedInputStream(in);
				String req_op2 = "Please enter the second operand: ";
				byteBuffer = req_op2.getBytes("UTF-8");
				out.write(byteBuffer);

				//Receive second operand
				op2 = bufop2.read();
				System.out.println("Client second operand: ");
				System.out.println(op2);
			}


			int result = 0;
			int error_code = 295; //127 in decimal

			if (opcode == 0 && numops == 2) {
				result = op1 + op2;
				error_code = 000;
			}

			else if (opcode == 1 && numops == 2) {
				result = op1 - op2;
				error_code = 000;
			}

			else if (opcode == 2 && numops == 2) {
				result = op1 | op2;
				error_code = 000;
			}

			else if (opcode == 3 && numops == 2) {
				result = op1 & op2;
				error_code = 000;
			}

			else if (opcode == 4 && numops == 2) {
				result = op1 >> op2;
				error_code = 000;
			}

			else if (opcode == 5 && numops == 2) {
				result = op1 << op2;
				error_code = 000;
			}

			else if (opcode == 6 && numops == 1) {
				result = ~op1;
				error_code = 000;
			}

			ByteBuffer b = ByteBuffer.allocate(4);

			//Send result TML
			int res_tml = 07;
			b.putInt(res_tml);
			byteBuffer = b.array();
			out.write(byteBuffer);

			//Send Request ID
			req_id++;
			System.out.println(req_id);
			out.write(req_id);

			//Send Error Code
			System.out.println(error_code);
			out.write(error_code);

			//Send result
			System.out.println(result);
			out.write(result);


			clntSock.close();
		}	
 	} /* NOT REACHED */
}