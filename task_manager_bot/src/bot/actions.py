from collections import namedtuple


BotAction = namedtuple(
    "BotAction",
    (
        "action_name",  # str
        "chat_id",  # int
        "args",  # dict
    ),
)
