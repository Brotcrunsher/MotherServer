import asyncio
import websockets
import nacl.utils
import base64
import hashlib
import ssl

with open("/home/MotherServer/hashedPw.txt", "r") as f:
    pw_hash = f.read()

# Define a callback function to handle incoming WebSocket messages
async def handle_websocket(websocket, path):
    try:
        while True:
            print("Waiting for new Message...")
            message = await websocket.recv()
            if message != "Give Challenge plz":
                print("Connection partner sent something unexpected.")
                await websocket.close()
                continue
            
            challenge = base64.b64encode(nacl.utils.random(size=32)).decode('ascii')
            print("Sending challenge...")
            await websocket.send(challenge)
            print("Challenge sent! Waiting for reply...")
            task = await websocket.recv()
            length = len(task)
            if length == 0:
              print("Received empty task.")
              await websocket.send("Empty Task!")
              await websocket.close()
              continue
            if length > 1024:
              print("Task Length sanity check exceeded with length: " + str(length))
              await websocket.send("Task too long!")
              await websocket.close()
              continue
            challenge_response_received = await websocket.recv()
            
            challenge_response_expected = hashlib.sha512((challenge + pw_hash + task).encode('utf-8')).hexdigest()
            
            print("Task: " + task)
            #print("Challenge Response Received: " + challenge_response_received)
            print("Challenge Response Expected: " + challenge_response_expected)
            
            # Todo: I believe this allows for a potential side channel attack?
            if challenge_response_received == challenge_response_expected:
              print("Writing to file...")
              with open("/home/MotherServer/Tasks.txt", "a") as f:
                f.write(challenge_response_expected + ":" + task + "\n")
              print("Success!")
              await websocket.send("All good!")
            else:
              print("Got bad challenge.")
              await websocket.send("Bad password!")
            
            print("Closing...")
            await websocket.close()
            print("Closed!")
    except websockets.ConnectionClosed:
        pass

if __name__ == "__main__":
    print("Starting Server...")
    ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    ssl_context.load_cert_chain("/home/MotherServer/domain.crt", keyfile="/home/MotherServer/domain.key")
    # Start the WebSocket server
    start_server = websockets.serve(handle_websocket, "CHANGE ME TO ADDRESS", 8765, ssl=ssl_context)
    
    print("Run until complete...")
    asyncio.get_event_loop().run_until_complete(start_server)
    print("Run forever...")
    asyncio.get_event_loop().run_forever()
    