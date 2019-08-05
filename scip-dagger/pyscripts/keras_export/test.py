# create_model.py
import numpy as np
import os
from keras.layers import Input, Dense
from keras.models import Model

inputs = Input(shape=(4,))
x = Dense(5, activation='relu')(inputs)
predictions = Dense(3, activation='softmax')(x)
model = Model(inputs=inputs, outputs=predictions)
model.compile(loss='categorical_crossentropy', optimizer='nadam')

model.fit(
    np.asarray([[1,2,3,4], [2,3,4,5]]),
    np.asarray([[1,0,0], [0,0,1]]), epochs=10)

out = model.predict(np.array([[1,2,3,4]]))
print(out)

model.save('keras_model.h5', include_optimizer=False)
os.system('python3 convert_model.py keras_model.h5 fdeep_model.json')

