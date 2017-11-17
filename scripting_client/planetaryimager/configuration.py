from .network import *


class Configuration:
    """Enumerates, get and set configuration values of Planetary Imager

    The available items that can be remotely configured by the scripting protocol can be seen with the `list` method.
    Note that some configuration items are C++ enums, which are translated in the network protocol as numbers.
    Refer to `src/commons/configuration.h` in PlanetaryImager to see the allowed values for enum entries.
    """

    class Entry:
        """Class representing a configuration entry"""
        def __init__(self, name, type, configuration_protocol):
            """This entry name."""
            self.name = name
            """The type in PlanetaryImager (QString, int, an enum type, etc)."""
            self.type = type
            self.configuration_protocol = configuration_protocol

        @property
        def value(self):
            """Get the current entry value"""
            return self.configuration_protocol.get(self.name)['value']

        @value.setter
        def value(self, value):
            """Set a new configuration value for this entry.

            :param value: new value to set.
            """
            self.configuration_protocol.set(self.name, value)

        def reset(self):
            """Reset this entry to its default configuration value."""
            self.configuration_protocol.reset(self.name)

        def __str__(self):
            return '{} ({}) = {}'.format(self.name, self.type, self.value)

        def __repr__(self):
            return self.__str__()

    def __init__(self, client):
        self.configuration_protocol = ConfigurationProtocol(client)
        self.__entries = None

    @property
    def entries(self):
        """List the available configuration entries.

        :return: a dictionary with the configuration key names as keys, and the type as values.
        """
        if not self.__entries:
            self.__entries = {}
            for name, type in self.configuration_protocol.list().items():
                self.__entries[name] = Configuration.Entry(name, type, self.configuration_protocol)
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

    def on_settings_changed(self, callback):
        """Run the provided callback when settings change."""
        self.configuration_protocol.on_signal_settings_changed(callback)

    def __getattr__(self, item):
        """Access a configuration item as an attribute (configuration.entry_name)"""
        return self.entries[item] if item in self.entries else None

    def __getitem__(self, item):
        """Access a configuration item as a dictionary element (configuration['entry_name'])"""
        return self.entries[item]
