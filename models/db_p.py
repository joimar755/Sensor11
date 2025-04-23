from sqlalchemy import TIMESTAMP, Integer, String, Table, Column, text, true, ForeignKey
from sqlalchemy.orm import relationship
from config.db import Base


class bombillo(Base):
    __tablename__ = "bombillo"
    id = Column(Integer, primary_key=True)
    led1 = Column(Integer, nullable=False)
    led2 = Column(Integer, nullable=False)
    led3 = Column(Integer, nullable=False)
    bloqueos = Column(Integer, default=0) 
    user_id = Column(Integer, ForeignKey("users.id", ondelete="CASCADE"), nullable=False)
    created_at = Column(TIMESTAMP(timezone=True), nullable=False, server_default=text('now()'))


    
    
class Datos(Base):
    __tablename__ = "Datos"
    id = Column(Integer, primary_key=True)
    valor = Column(String(255), nullable=False)
    user_id = Column(Integer, ForeignKey("users.id", ondelete="CASCADE"), nullable=False)
    created_at = Column(TIMESTAMP(timezone=True), nullable=False, server_default=text('now()'))
    

class Users(Base):
    __tablename__ = "users"
    id = Column(Integer, primary_key=True)
    username = Column(String(255), nullable=False, unique=True)
    password = Column(String(255), nullable=False)
    phone_number = Column(String(12))
    created_at = Column(TIMESTAMP(timezone=True), nullable=False, server_default=text('now()'))

    



