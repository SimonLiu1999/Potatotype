from flask import Flask,jsonify,request,render_template
app = Flask(__name__)
@app.route('/')
def hello():
    return render_template("index.html")


@app.route('/index', methods=['POST'])
def index():
    sentence = request.form['sentence']
    res = str(eval(sentence))

    return jsonify({'sentence': res})

if __name__ == '__main__':
    app.run() #127.0.0.1 回路 自己返回自己