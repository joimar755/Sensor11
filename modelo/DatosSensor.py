# modelo/m_pro.py

from datetime import datetime
from typing import Optional
from pydantic import BaseModel

# Este solo lo usas para recibir datos desde el ESP32
class DatosCreate(BaseModel):
    valor: str


# Este para retornar o consultar (ya con los campos extra)
class DatosOut(DatosCreate):
    id: int
    user_id: int
    created_at: datetime

    class Config:
        from_attributes = True
