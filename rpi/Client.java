import java.io.*;
import java.net.*;
import java.util.*;
public class Client {
	private static Socket socket = null;
	public Client(){}

	public static void setUp(String hostName, int portNumber){
		try{
			socket = new Socket(hostName, portNumber);
		} catch(Exception e){
			System.out.println("Socket error: " + e.getMessage());
		}
	}

	public static void write(String msg){
		PrintWriter out = null;
		try{
			out = new PrintWriter(socket.getOutputStream());
			System.out.println("to be sent:"+msg);
			out.print(msg);
			out.flush();
		} catch (Exception e){
			System.out.println("Write error: " + e.getMessage());
		}
	}

	public static String read(){
		BufferedReader receiveBuf = null;
		String inStr = null;
		try{
			receiveBuf = new BufferedReader(new InputStreamReader(socket.getInputStream()));
			inStr = receiveBuf.readLine();  
		}catch(Exception e){
			System.out.println("Read error: " + e.getMessage());
		}
		return inStr;
	}

	public static void main(String[] args){
		// SET UP SOCKET CONNECTION
		setUp("localhost", 5000);

		// READ AND WRITE
		Scanner sc = new Scanner(System.in);
		while (true){
			// write to socket
			System.out.print("Character sent to arduino: ");
			String msg = sc.nextLine();
			write(msg);

			//read from socket
			String inStr = read();  
			System.out.println("Received:"+inStr);

		}
	}
}