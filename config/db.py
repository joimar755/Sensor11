from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
import os 
from dotenv import load_dotenv

load_dotenv()

DATABASE_URL_1 = os.getenv("DATABASE_URL_1")
SQLALCHEMY_DATABASE_URL = "mysql+pymysql://ucyfzmysg55hlcut:5KdrGWiVUrTvb9uCJUbU@bgwito1c36qmhwkekquc-mysql.services.clever-cloud.com:3306/bgwito1c36qmhwkekquc"
engine = create_engine(
    #DATABASE_URL_1
    SQLALCHEMY_DATABASE_URL
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



