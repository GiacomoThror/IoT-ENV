from sqlalchemy import Column, ForeignKey, Integer, String, DateTime, Float

from sqlalchemy.ext.declarative import declarative_base

from sqlalchemy.orm import relationship

from sqlalchemy import create_engine


Base = declarative_base()


class WheaterStation(Base):

    __tablename__ = 'wheater_station'

    name = Column(String(80), nullable=False)
    id = Column(Integer, primary_key=True)

    @property
    def serialize(self):
        """Return object data in easily serializeable format"""
        return {
            'id': self.id,
            'name': self.name
        }


class Sensor(Base):

    __tablename__ = 'sensor'

    id = Column(Integer, primary_key=True)
    name = Column(String(80), nullable=False)
    weather_station_id = Column(Integer, ForeignKey('wheater_station.id'))

    weather_station = relationship(WheaterStation)

    # We added this serialize function to be able to send JSON objects in a
    # serializable format
    @property
    def serialize(self):
        return {
            'id': self.id,
            'name': self.name,
            # 'weather_station': weather_station_id
        }


class Value(Base):

    __tablename__ = 'value'

    id = Column(Integer, primary_key=True)
    id_sensor = Column(Integer, ForeignKey('sensor.id'))
    datetime = Column(DateTime, primary_key=True)
    value = Column(Float, primary_key=True)

    sensor = relationship(Sensor)

    # We added this serialize function to be able to send JSON objects in a
    # serializable format
    @property
    def serialize(self):
        return {
            'id': self.id,
            'sensor': id_sensor,
            'datetime': datetime,
            'value': value
        }


# INSERT AT END OF FILE #

engine = create_engine('sqlite:///weather_station.db')

Base.metadata.create_all(engine)
