import os


def get_env(env: str, default="", needed: bool = False):
    """
    if needed is True, default will be ignored
    """

    PREFIX = "BOT_"
    if env_var := os.environ.get(PREFIX + env):
        env_type = type(default)
        return env_type(env_var)
    elif needed:
        raise ValueError(f"required env var {env} has not been set")
    return default


BOT_TOKEN = get_env("TOKEN", needed=True)
API_ID = get_env("API_ID", needed=True)
API_HASH = get_env("API_HASH", needed=True)


ADMIN_ID = get_env("ADMIN_ID", 0, needed=True)
