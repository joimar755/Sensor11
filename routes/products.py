from datetime import timedelta
from typing import List
from fastapi import APIRouter, Depends, HTTPException, Response
from fastapi.encoders import jsonable_encoder
from fastapi.responses import JSONResponse
from fastapi.security.oauth2 import OAuth2PasswordRequestForm
from config.db import  SessionLocal, get_db
from passlib.context import CryptContext
from modelo import oauth
from modelo.oauth import get_current_user
from models.db_p import  bombillo, Users,Datos
from sqlalchemy.orm import Session
from models import db_p
from modelo import m_pro, DatosSensor
from modelo.m_user import Login, Token, users
from modelo.token import create_access_token
import json

VH = APIRouter()


pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")



@VH.get("/api/motion/data/bombillos")                                                                                                                                                                
def getproduct(db: Session = Depends(get_db)):
    product = db.query(bombillo).all()
    return {'data': product}


@VH.post("/api/motion/data/bombillos")
def getnew(
    vhs: m_pro.vhcreate,
    db: Session = Depends(get_db), current_user: int = Depends(oauth.get_current_user)
):

    #existe = db.query(bombillo).filter(bombillo.name_product == vhs.name_product).first()
    
    #print(current_user.id)     
    db_item = bombillo(user_id=current_user.id,**vhs.model_dump())
    db.add(db_item)
    db.commit()
    db.refresh(db_item)
    return {"data": db_item}
    

@VH.get("/api/motion/data/bombillos/status")
def get_estado_leds(
    db: Session = Depends(get_db),
    current_user: int = Depends(oauth.get_current_user)
):
    item = db.query(bombillo).filter(bombillo.user_id == current_user.id).order_by(bombillo.id.desc()).first()
    return {"led1": item.led1, "led2": item.led2, "led3": item.led3,"bloqeos":item.bloqueos}


@VH.post("/api/motion/data/create")
def getsensor(
    vhs_datos: DatosSensor.DatosCreate,
    db: Session = Depends(get_db),
    current_user: int = Depends(oauth.get_current_user)
):
    # Buscar el último bombillo asociado al usuario
    bombillo_obj = (
        db.query(bombillo)
        .filter(bombillo.user_id == current_user.id)
        .order_by(bombillo.id.desc())
        .first()
    )

    if not bombillo_obj:
        raise HTTPException(status_code=404, detail="No hay bombillos registrados para este usuario")

    # Crear el dato asociado a ese bombillo
    db_item = Datos(
        user_id=current_user.id,
        bombillo_id=bombillo_obj.id,
        valor=vhs_datos.valor  # Solo se espera 'valor' en el JSON
    )
    db.add(db_item)
    db.commit()
    db.refresh(db_item)
    return {"data": db_item}



    

@VH.get("/api/motion/estado")
def estado_actual(db: Session = Depends(get_db)):
    ultimo = db.query(bombillo).order_by(bombillo.id.desc()).first()
    return {
        "led1": ultimo.led1,
        "led2": ultimo.led2,
        "led3": ultimo.led3
    }


@VH.get("/products/{id}")
def index(id: int, db: Session = Depends(get_db)):
    vh = db.query(bombillo).filter(bombillo.id == id).first()
    if vh is None:
        raise HTTPException(status_code=404, detail="Product not found")
    return vh


@VH.get("/relacion", response_model=List[m_pro.vh])
def index(db: Session = Depends(get_db)):
    query = (
        db.query(Vehiculos, Category, Model_Auto)
        .join(Category, Vehiculos.category_id == Category.id)
        .join(Model_Auto, Vehiculos.modelo_id == Model_Auto.id)
        .all()
    )

    # Iterar y mostrar los resultados
    resultados = [
        {
            "id": vehiculo.id,
            "name_product": vehiculo.name_product,
            "category": categoria.name,
            "price":vehiculo.price,
            "modelo": modelo.modelo,
        }
        for vehiculo, categoria, modelo in query
    ]

    # Serializar la respuesta usando jsonable_encoder y devolver como JSONResponse
    return JSONResponse(content=jsonable_encoder(resultados))


@VH.delete("/products/{id}")
def delete(id: int, db: Session = Depends(get_db),current_user: int = Depends(oauth.get_current_user)):
    vh_query = db.query(Vehiculos).filter(Vehiculos.id == id)
    vh = vh_query.first()
    if vh == None:
        raise HTTPException(status_code=404, detail="Product not found")

    if vh.user_id != current_user.id:
        raise HTTPException(status_code=401, detail="Unauthorized to delete this product") 

    vh.delete(synchronize_session=False)
    db.commit()
    raise HTTPException(status_code=200, detail="Product delete")

@VH.get("/profile/{id}")
def index(id: int, db: Session = Depends(get_db),current_user: int = Depends(oauth.get_current_user)):
    vh_query = db.query(Users).filter(Users.id == id)
    vh = vh_query.first()
    if vh == None:
        raise HTTPException(status_code=404, detail="user not found")
    if vh.id != current_user.id:
        raise HTTPException(status_code=401, detail="Unauthorized to users") 
    
    return {"user":vh}

@VH.put("/products/{id}", response_model=m_pro.vhBase)
def index(id: int, update_vhs: m_pro.vhBase ,current_user: int = Depends(get_current_user), db: Session = Depends(get_db)):
    vh_query = db.query(bombillo).filter(bombillo.id == id)
    post_vh = vh_query.first()
    if not vh_query.first():
        raise HTTPException(status_code=404, detail="Product not found")
    
    if post_vh.user_id != current_user.id:
        raise HTTPException(status_code=401, detail="Unauthorized to update this product") 

    vh_query.update(update_vhs.model_dump(), synchronize_session=False)
    db.commit()
    db.refresh(post_vh)
    return post_vh


@VH.post("/usuario")
def get_user(user: users,db: Session = Depends(get_db)):
    existe = db.query(Users).filter(Users.username == user.username).first()
    if existe:
        return JSONResponse("usuario ya se encuentra en uso")
    if existe is None:
            hashed_password = pwd_context.hash(user.password)
            user.password = hashed_password
            db_item = Users(**user.model_dump())
            db.add(db_item)
            db.commit()
            db.refresh(db_item)
    else:
        raise HTTPException(
            status_code=404, detail="product with this name already exists "
         )
    #vh_query = db.query(Users).filter(Users.id == db_item.id).first()
    return  db_item


@VH.post("/usuario/login")
def get_user(user_credentials:OAuth2PasswordRequestForm=Depends(),db: Session = Depends(get_db)):
    user = db.query(Users).filter(Users.username == user_credentials.username).first()

    if not user or not pwd_context.verify(user_credentials.password, user.password):
        return JSONResponse("Incorrect username or password")
        raise HTTPException(409, "Incorrect username or password")

    access_token =  create_access_token(data={"user_id": user.id})
    token = {"access_token": access_token, "username":user.username, "userID":user.id, "token_type": "bearer"}
    return JSONResponse(token)



# raise HTTPException(status_code=200, detail="login")
