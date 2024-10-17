import sys
import pygame
import asyncio
async def handle_input():
    global value
    value = 0
    loop = asyncio.get_event_loop()
    reader = asyncio.StreamReader(loop=loop)
    reader_protocol = asyncio.StreamReaderProtocol(reader)

    await loop.connect_read_pipe(lambda: reader_protocol, sys.stdin)

    while True:
        data = await reader.readline()
        print(data.decode().rstrip())
        value = abs(int(float(data.decode().rstrip())))
        if value > 765:
            value = 765 - (value/765)
async def main():
    # Initialize Pygame here
    pygame.init()

    input_task = asyncio.create_task(handle_input())
    pygame_task = loop.run_in_executor(None, pygame_loop)

    await asyncio.gather(input_task, pygame_task)

    pygame.quit()
    sys.exit()


def pygame_loop():
    global value
    width, height = 500, 300
    #  video = cv2.VideoCapture("rat2.mp4")
    screen = pygame.display.set_mode((width, height))
    pygame.display.set_caption("audio vis")
    clock = pygame.time.Clock()
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        if value <= 255:
            screen.fill((0, 0, int(value)))
        if value > 255 and value <= 510:
            screen.fill((0, int(value)-255, 255))
        if value > 510:
            screen.fill((int(value)-510,255,255))
        #  success, video_image = video.read()
        #  if success:
            #  video_surf = pygame.image.frombuffer(video_image.tobytes(), video_image.shape[1::-1], "BGR")
        #  else: 
            #  try:
                #  video = cv2.VideoCapture("rat2.mp4")
            #  except Exception as e:
                #  print(e)
        #  screen.blit(video_surf, (0,0))
        pygame.display.flip()
        if value > 10:
            clock.tick(1000)
        else:
            # idle
            clock.tick(15)
        # clock.tick(min(max(20, int(val/2)),120))

if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())

