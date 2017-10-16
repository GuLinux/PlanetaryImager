from .network import *

class Configuration:
    """Enumerates, get and set configuration values of Planetary Imager

    The available items that can be remotely configured by the scripting protocol can be seen with the `list` method.
    Note that some configuration items are C++ enums, which are translated in the network protocol as numbers.
    Refer to `src/commons/configuration.h` in PlanetaryImager to see the allowed values for enum entries.
    """
    def __init__(self, client):
        self.client = client

    def list(self):
        """List the available configuration entries.

        :return: a dictionary with the configuration key names as keys, and the type as values.
        """
        return ConfigurationProtocol.list(self.client)

    def get(self, name):
        """Retrieve the current configuration value for `name`.

        :param name: the configuration item name
        :return: a dictionary populated with configuration `name` and `value`.
        """
        return ConfigurationProtocol.get(self.client, name)

    def set(self, name, value):
        """Set a new configuration value for `name`.

        :param name: name of the configuration item to set.
        :param value: new value to set.
        """
        ConfigurationProtocol.set(self.client, name, value)

    def reset(self, name):
        """Reset `name` to its default configuration value."""
        ConfigurationProtocol.reset(self.client, name)

