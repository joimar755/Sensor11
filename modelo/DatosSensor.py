# modelo/m_pro.py

from typing import Optional
from pydantic import BaseModel

# Este solo lo usas para recibir datos desde el ESP32
class DatosCreate(BaseModel):
    valor: str
    bombillo_id: Optional[int] = None  # Ahora es opcional

# Este para retornar o consultar (ya con los campos extra)
class DatosOut(DatosCreate):
    id: int
    user_id: int
    created_at: str

    class Config:
        from_attributes = True