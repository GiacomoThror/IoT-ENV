from sqlalchemy import Column, ForeignKey, Integer, String

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
            'name': self.name,
            'id': self.id,
        }


class Sensor(Base):

    __tablename__ = 'sensors'

    name = Column(String(80), nullable=False)
    id = Column(Integer, primary_key=True)
    weather_station_id = Column(Integer, ForeignKey('wheater_station.id'))

    weather_station = relationship(WheaterStation)

    # We added this serialize function to be able to send JSON objects in a
    # serializable format
    @property
    def serialize(self):
        return {
            'name': self.name,
            'id': self.id,
        }


# INSERT AT END OF FILE #

engine = create_engine('sqlite:///weather_station.db')

Base.metadata.create_all(engine)
