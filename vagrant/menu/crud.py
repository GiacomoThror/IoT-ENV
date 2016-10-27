#!/usr/bin/env python

from sqlalchemy import create_engine

from sqlalchemy.orm import sessionmaker

from database_setup import Base, Restaurant, MenuItem

engine = create_engine('sqlite:///restaurantmenu.db')

Base.metadata.bind = engine
DBSession = sessionmaker(bind=engine)
session = DBSession()

# INSERT #

# myFirstRestaurant =  Restaurant(name= "Pizza Palace")

# session.add(myFirstRestaurant)
# session.commit()

# cheesePizza = MenuItem(name="Cheese Pizza", course="Entree",
# description="Made with all natural ingredients and fresh mozzarella",
#  price="$8.99", restaurant=myFirstRestaurant)

# session.add(cheesePizza)
# session.commit()

# print(session.query(cheesePizza).all())

# READ #

# items = session.query(Restaurant).all()

# for item in items:

# print(item.name)

# UPDATE #

UrbanVeggieBurger = session.query(MenuItem).filter_by(id=9).one()

if UrbanVeggieBurger.price != '$2.99':

	print (UrbanVeggieBurger.price)
	UrbanVeggieBurger.price = '$2.99'


session.add(UrbanVeggieBurger)
session.commit()


veggieBurgers = session.query(MenuItem).filter_by(name='Veggie Burger')

for burger in veggieBurgers:
	print(burger.id)
	print(burger.price)
	print(burger.restaurant.name)
	print '\n'


# DELETE #

spinach = session.query(MenuItem).filter_by(name='Spinach Ice Cream').one()
# print spinach.description

session.delete(spinach)
session.commit()
