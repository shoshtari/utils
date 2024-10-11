import os

import telethon

from . import configs
from .actions import BotAction
from src.message_handler import handle_message


def run():

    client = telethon.TelegramClient(
        "bot_session",
        configs.API_ID,
        configs.API_HASH,
        proxy=("socks5", "127.0.0.1", 2080),
    ).start(bot_token=configs.BOT_TOKEN)

    @client.on(telethon.events.NewMessage)
    async def message_handler(event: telethon.events.newmessage.NewMessage.Event):
        handle_message(event.chat_id, event.message)

    async def carry_action(action: BotAction):
        match action.action_name:
            case "send_message":
                assert "text" in action.args, "send message action needs text"
                await client.send_message(action.chat_id, action.args["text"])
            case _:
                raise NotImplementedError()

    async def send_hello_message():
        chat_id = configs.ADMIN_ID
        message = "Hello from your bot!"

        await client.send_message(chat_id, message)

    # Call the function to send the message
    print("running bot...")
    client.run_until_disconnected()
