from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
import os 
from dotenv import load_dotenv

load_dotenv()

DATABASE_URL = os.getenv("SQLALCHEMY_DATABASE_URL_SENSOR")
#SQLALCHEMY_DATABASE_URL = "mysql+mysqlconnector://root:@localhost:3306/Vehiculos"
engine = create_engine(
    DATABASE_URL
    #SQLALCHEMY_DATABASE_URL
)
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()
#Base.metadata.create_all(bind=engine, checkfirst=True)

def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()



