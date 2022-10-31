import logging
import sys

LOG_FORMAT  = "[%(asctime)s] %(name)s | %(levelname)s: %(message)s"
TIME_FORMAT = "%H:%M:%S"

logger    = logging.getLogger("cpak")
handler   = logging.StreamHandler(sys.stdout)
formatter = logging.Formatter(LOG_FORMAT, TIME_FORMAT)
handler.setFormatter(formatter)
logger.addHandler(handler)
logger.setLevel(logging.INFO)
