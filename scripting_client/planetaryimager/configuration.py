from .network import *


class Configuration:
    """Enumerates, get and set configuration values of Planetary Imager

    The available items that can be remotely configured by the scripting protocol can be seen with the `list` method.
    Note that some configuration items are C++ enums, which are translated in the network protocol as numbers.
    Refer to `src/commons/configuration.h` in PlanetaryImager to see the allowed values for enum entries.
    """

    class Entry:
        """Class representing a configuration entry"""
        def __init__(self, name, type, client):
            """This entry name."""
            self.name = name
            """The type in PlanetaryImager (QString, int, an enum type, etc)."""
            self.type = type
            self.client = client

        @property
        def value(self):
            """Get the current entry value"""
            return ConfigurationProtocol.get(self.client, self.name)['value']

        @value.setter
        def value(self, value):
            """Set a new configuration value for this entry.

            :param value: new value to set.
            """
            ConfigurationProtocol.set(self.client, self.name, value)

        def reset(self):
            """Reset this entry to its default configuration value."""
            ConfigurationProtocol.get(self.client, self.name)

        def __str__(self):
            return '{} ({}) = {}'.format(self.name, self.type, self.value)

        def __repr__(self):
            return self.__str__()

    def __init__(self, client):
        self.client = client
        self.__entries = None

    @property
    def entries(self):
        """List the available configuration entries.

        :return: a dictionary with the configuration key names as keys, and the type as values.
        """
        if not self.__entries:
            self.__entries = {}
            for name, type in ConfigurationProtocol.list(self.client).items():
                self.__entries[name] = Configuration.Entry(name, type, self.client)
        return self.__entries

    @property
    def keys(self):
        """List all the configuration entries by name"""
        return list(self.entries.keys())

    def entry(self, name):
        """Retrieve the current configuration object for `name`.

        :param name: the configuration item name
        :return: a Configuration.Entry object.
        """
        return self.entries[name]

    def __getattr__(self, item):
        """Access a configuration item as an attribute (configuration.entry_name)"""
        return self.entries[item] if item in self.entries else None

    def __getitem__(self, item):
        """Access a configuration item as a dictionary element (configuration['entry_name'])"""
        return self.entries[item]
