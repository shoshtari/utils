import os

import telethon

from . import configs


def run():

    client = telethon.TelegramClient(
        "bot_session",
        configs.API_ID,
        configs.API_HASH,
        proxy=("socks5", "127.0.0.1", 2080),
    ).start(bot_token=configs.BOT_TOKEN)

    async def send_hello_message():
        chat_id = configs.ADMIN_ID  # Replace with the chat ID of the user or group
        message = "Hello from your bot!"

        await client.send_message(chat_id, message)

    # Call the function to send the message
    client.loop.run_until_complete(send_hello_message())
