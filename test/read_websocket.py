import asyncio
import websockets

async def read_websocket():
<<<<<<< Updated upstream
    async with websockets.connect("ws://localhost:9002") as websocket:
=======
    async with websockets.connect("ws://127.0.0.1:9000") as websocket:
>>>>>>> Stashed changes
        i = 0
        while True:
            message = await websocket.recv()
            if i % 100 == 0:
                print(message)

asyncio.run(read_websocket())